/* main.c - Application main entry point */

/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

extern "C" {

#include <stdbool.h>
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <zephyr.h>

/**
 * Line 600 of bluetooth.h
 * need to add a const to struct array to compile
 * #define BT_LE_ADV_PARAM(_options, _int_min, _int_max, _peer) \
	((const struct bt_le_adv_param[]) { \
		BT_LE_ADV_PARAM_INIT(_options, _int_min, _int_max, _peer) \
	 })
 */
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/bas.h>
}

#include <humidity/humidity_sensor.hpp>
#include <connection_manager/connection_manager.hpp>
#include <services/ess/environmental_sensing_service.hpp>

#define SENSOR_1_NAME        "Temperature Sensor 1"
#define SENSOR_2_NAME        "Temperature Sensor 2"
#define SENSOR_3_NAME        "Humidity Sensor"

/* Sensor Internal Update Interval [seconds] */
#define SENSOR_1_UPDATE_IVAL      5
#define SENSOR_2_UPDATE_IVAL      12
#define SENSOR_3_UPDATE_IVAL      60

bt::ess::EnvironmentalSensingService temp_sensor{
    "Temperature Sensor CPP",
    bt::ess::Measurement{
        bt::ess::Measurement::SamplingFunction::unspecified,
        bt::ess::Measurement::Application::indoor,
        1,
        1,
        5
    },
    bt::ess::TriggerSetting{
        bt::ess::TriggerSetting::Condition::value_changed
    },
    bt::ess::Configuration{
        bt::ess::Configuration::ClientCharConfig::none
    }
};

/* ESS error definitions */
#define ESS_ERR_WRITE_REJECT      0x80
#define ESS_ERR_COND_NOT_SUPP      0x81

/* ESS Trigger Setting conditions */
#define ESS_TRIGGER_INACTIVE      0x00
#define ESS_FIXED_TIME_INTERVAL      0x01
#define ESS_NO_LESS_THAN_SPECIFIED_TIME    0x02
#define ESS_VALUE_CHANGED      0x03
#define ESS_LESS_THAN_REF_VALUE      0x04
#define ESS_LESS_OR_EQUAL_TO_REF_VALUE    0x05
#define ESS_GREATER_THAN_REF_VALUE    0x06
#define ESS_GREATER_OR_EQUAL_TO_REF_VALUE  0x07
#define ESS_EQUAL_TO_REF_VALUE      0x08
#define ESS_NOT_EQUAL_TO_REF_VALUE    0x09

static ssize_t read_u16_cpp(struct bt_conn* conn, const struct bt_gatt_attr* attr,
    void* buf, uint16_t len, uint16_t offset)
{
  const auto service =
      static_cast<bt::ess::EnvironmentalSensingService*>(attr->user_data);
  return service->read_value(conn, attr, buf, len, offset);
}

static ssize_t read_u16(struct bt_conn* conn, const struct bt_gatt_attr* attr,
    void* buf, uint16_t len, uint16_t offset)
{
  const uint16_t* u16 = static_cast<const uint16_t*>(attr->user_data);
  uint16_t value = sys_cpu_to_le16(*u16);

  printk("reading u16: %d\n", value);

  return bt_gatt_attr_read(conn, attr, buf, len, offset, &value,
      sizeof(value));
}

/* Environmental Sensing Service Declaration */

struct es_measurement {
  uint16_t flags; /* Reserved for Future Use */
  uint8_t sampling_func;
  uint32_t meas_period;
  uint32_t update_interval;
  uint8_t application;
  uint8_t meas_uncertainty;
};

struct temperature_sensor {
  int16_t temp_value;

  /* Valid Range */
  int16_t lower_limit;
  int16_t upper_limit;

  /* ES trigger setting - Value Notification condition */
  uint8_t condition;
  union {
    uint32_t seconds;
    int16_t ref_val; /* Reference temperature */
  };

  struct es_measurement meas;
};

struct humidity_sensor {
  int16_t humid_value;

  struct es_measurement meas;
};

static bool simulate_temp;
static struct temperature_sensor sensor_1 = {
    .temp_value = 1200,
    .lower_limit = -10000,
    .upper_limit = 10000,
    .condition = ESS_VALUE_CHANGED,
    .meas = {
        .sampling_func = 0x00,
        .meas_period = 0x01,
        .update_interval = SENSOR_1_UPDATE_IVAL,
        .application = 0x1c,
        .meas_uncertainty = 0x04,
    }
};

static struct temperature_sensor sensor_2 = {
    .temp_value = 1800,
    .lower_limit = -1000,
    .upper_limit = 5000,
    .condition = ESS_VALUE_CHANGED,
    .meas = {
        .sampling_func = 0x00,
        .meas_period = 0x01,
        .update_interval = SENSOR_2_UPDATE_IVAL,
        .application = 0x1b,
        .meas_uncertainty = 0x04,
    }
};

static struct humidity_sensor sensor_3 = {
    .humid_value = 6233,
    .meas = {
        .sampling_func = 0x02,
        .meas_period = 0x0e10,
        .update_interval = SENSOR_3_UPDATE_IVAL,
        .application = 0x1c,
        .meas_uncertainty = 0x01,
    }
};

static void temp_ccc_cfg_changed(const struct bt_gatt_attr* attr,
    uint16_t value)
{
  printk("temp ccc cfg changed");
  simulate_temp = value == BT_GATT_CCC_NOTIFY;
}

static void es_ccc_cfg_changed_cpp(const struct bt_gatt_attr* attr,
    uint16_t value)
{
  bt::ess::Configuration::ClientCharConfig config
      = bt::ess::Configuration::ClientCharConfig::none;
  switch (value) {
    case 0:
      config = bt::ess::Configuration::ClientCharConfig::none;
    case BT_GATT_CCC_NOTIFY:
      config = bt::ess::Configuration::ClientCharConfig::notify;
    case BT_GATT_CCC_INDICATE:
      config = bt::ess::Configuration::ClientCharConfig::indicate;
  }

  auto service =
      static_cast<bt::ess::EnvironmentalSensingService*>(attr->user_data);
  return service->get_configuration().set_ccc(config);
}

struct read_es_measurement_rp {
  uint16_t flags; /* Reserved for Future Use */
  uint8_t sampling_function;
  uint8_t measurement_period[3];
  uint8_t update_interval[3];
  uint8_t application;
  uint8_t measurement_uncertainty;
} __packed;

static ssize_t read_es_measurement(struct bt_conn* conn,
    const struct bt_gatt_attr* attr, void* buf,
    uint16_t len, uint16_t offset)
{
  const struct es_measurement* value = static_cast<const es_measurement*>(attr->user_data);
  struct read_es_measurement_rp rsp;

  rsp.flags = sys_cpu_to_le16(value->flags);
  rsp.sampling_function = value->sampling_func;
  sys_put_le24(value->meas_period, rsp.measurement_period);
  sys_put_le24(value->update_interval, rsp.update_interval);
  rsp.application = value->application;
  rsp.measurement_uncertainty = value->meas_uncertainty;

  return bt_gatt_attr_read(conn, attr, buf, len, offset, &rsp,
      sizeof(rsp));
}

static ssize_t read_es_measurement_cpp(struct bt_conn* conn,
    const struct bt_gatt_attr* attr, void* buf,
    uint16_t len, uint16_t offset)
{
  const auto service =
      static_cast<bt::ess::EnvironmentalSensingService*>(attr->user_data);
  return service->get_measurement().read_measurement(conn, attr, buf, len,
      offset);
}

static ssize_t read_temp_valid_range(struct bt_conn* conn,
    const struct bt_gatt_attr* attr, void* buf,
    uint16_t len, uint16_t offset)
{
  const struct temperature_sensor* sensor = static_cast<const temperature_sensor*>(attr->user_data);
  uint16_t tmp[] = {sys_cpu_to_le16(sensor->lower_limit),
                    sys_cpu_to_le16(sensor->upper_limit)};

  return bt_gatt_attr_read(conn, attr, buf, len, offset, tmp,
      sizeof(tmp));
}

static ssize_t read_es_valid_range_cpp(struct bt_conn* conn,
    const struct bt_gatt_attr* attr, void* buf,
    uint16_t len, uint16_t offset)
{
  const auto service =
      static_cast<bt::ess::EnvironmentalSensingService*>(attr->user_data);
  return service->get_valid_range().read_valid_range(conn, attr, buf, len,
      offset);
}

struct es_trigger_setting_seconds {
  uint8_t condition;
  uint8_t sec[3];
} __packed;

struct es_trigger_setting_reference {
  uint8_t condition;
  int16_t ref_val;
} __packed;

static ssize_t read_temp_trigger_setting(struct bt_conn* conn,
    const struct bt_gatt_attr* attr,
    void* buf, uint16_t len,
    uint16_t offset)
{
  const struct temperature_sensor* sensor = static_cast<const temperature_sensor*>(attr->user_data);

  switch (sensor->condition) {
    /* Operand N/A */
    case ESS_TRIGGER_INACTIVE:
          __fallthrough;
    case ESS_VALUE_CHANGED:
      return bt_gatt_attr_read(conn, attr, buf, len, offset,
          &sensor->condition,
          sizeof(sensor->condition));
      /* Seconds */
    case ESS_FIXED_TIME_INTERVAL:
          __fallthrough;
    case ESS_NO_LESS_THAN_SPECIFIED_TIME: {
      struct es_trigger_setting_seconds rp;

      rp.condition = sensor->condition;
      sys_put_le24(sensor->seconds, rp.sec);

      return bt_gatt_attr_read(conn, attr, buf, len, offset,
          &rp, sizeof(rp));
    }
      /* Reference temperature */
    default: {
      struct es_trigger_setting_reference rp;

      rp.condition = sensor->condition;
      rp.ref_val = sys_cpu_to_le16(sensor->ref_val);

      return bt_gatt_attr_read(conn, attr, buf, len, offset,
          &rp, sizeof(rp));
    }
  }
}

static ssize_t read_es_trigger_setting_cpp(struct bt_conn* conn,
    const struct bt_gatt_attr* attr,
    void* buf, uint16_t len,
    uint16_t offset)
{
  const auto service =
      static_cast<bt::ess::EnvironmentalSensingService*>(attr->user_data);
  return service->get_trigger_setting().read_trigger_setting(conn, attr, buf, len,
      offset);
}

static bool check_condition(uint8_t condition, int16_t old_val, int16_t new_val,
    int16_t ref_val)
{
  switch (condition) {
    case ESS_TRIGGER_INACTIVE:
      return false;
    case ESS_FIXED_TIME_INTERVAL:
    case ESS_NO_LESS_THAN_SPECIFIED_TIME:
      /* TODO: Check time requirements */
      return false;
    case ESS_VALUE_CHANGED:
      return new_val != old_val;
    case ESS_LESS_THAN_REF_VALUE:
      return new_val < ref_val;
    case ESS_LESS_OR_EQUAL_TO_REF_VALUE:
      return new_val <= ref_val;
    case ESS_GREATER_THAN_REF_VALUE:
      return new_val > ref_val;
    case ESS_GREATER_OR_EQUAL_TO_REF_VALUE:
      return new_val >= ref_val;
    case ESS_EQUAL_TO_REF_VALUE:
      return new_val == ref_val;
    case ESS_NOT_EQUAL_TO_REF_VALUE:
      return new_val != ref_val;
    default:
      return false;
  }
}

static void update_temperature(struct bt_conn* conn,
    const struct bt_gatt_attr* chrc, int16_t value,
    struct temperature_sensor* sensor)
{
  bool notify = check_condition(sensor->condition,
      sensor->temp_value, value,
      sensor->ref_val);

  /* Update temperature value */
  sensor->temp_value = value;

  /* Trigger notification if conditions are met */
  if (notify) {
    value = sys_cpu_to_le16(sensor->temp_value);

    printk("sending notification: %d\n", value);
    bt_gatt_notify(conn, chrc, &value, sizeof(value));
  }
}

BT_GATT_SERVICE_DEFINE(ess_svc,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_ESS),

/* Temperature Sensor 1 */
    BT_GATT_CHARACTERISTIC(BT_UUID_TEMPERATURE,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
        BT_GATT_PERM_READ,
        read_u16, NULL, &sensor_1.temp_value),

    BT_GATT_DESCRIPTOR(BT_UUID_ES_MEASUREMENT, BT_GATT_PERM_READ,
        read_es_measurement, NULL, &sensor_1.meas),

    BT_GATT_CUD(SENSOR_1_NAME, BT_GATT_PERM_READ),

    BT_GATT_DESCRIPTOR(BT_UUID_VALID_RANGE, BT_GATT_PERM_READ,
        read_temp_valid_range, NULL, &sensor_1),

    BT_GATT_DESCRIPTOR(BT_UUID_ES_TRIGGER_SETTING,
        BT_GATT_PERM_READ, read_temp_trigger_setting,
        NULL, &sensor_1),

    BT_GATT_CCC(temp_ccc_cfg_changed,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

/* Temperature Sensor 2 */
    BT_GATT_CHARACTERISTIC(BT_UUID_TEMPERATURE,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
        BT_GATT_PERM_READ,
        read_u16, NULL, &sensor_2.temp_value),

    BT_GATT_DESCRIPTOR(BT_UUID_ES_MEASUREMENT, BT_GATT_PERM_READ,
        read_es_measurement, NULL, &sensor_2.meas),

    BT_GATT_CUD(SENSOR_2_NAME, BT_GATT_PERM_READ),

    BT_GATT_DESCRIPTOR(BT_UUID_VALID_RANGE, BT_GATT_PERM_READ,
        read_temp_valid_range, NULL, &sensor_2),
    BT_GATT_DESCRIPTOR(BT_UUID_ES_TRIGGER_SETTING,
        BT_GATT_PERM_READ, read_temp_trigger_setting,
        NULL, &sensor_2),
    BT_GATT_CCC(temp_ccc_cfg_changed,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

    /* Temperature Sensor cpp */
    BT_GATT_CHARACTERISTIC(BT_UUID_TEMPERATURE,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
        BT_GATT_PERM_READ,
        read_u16_cpp, NULL, &temp_sensor),

    BT_GATT_DESCRIPTOR(BT_UUID_ES_MEASUREMENT, BT_GATT_PERM_READ,
        read_es_measurement_cpp, NULL, &temp_sensor),

    BT_GATT_CUD(temp_sensor.get_name(), BT_GATT_PERM_READ),

    BT_GATT_DESCRIPTOR(BT_UUID_VALID_RANGE, BT_GATT_PERM_READ,
        read_es_valid_range_cpp, NULL, &temp_sensor),

    BT_GATT_DESCRIPTOR(BT_UUID_ES_TRIGGER_SETTING,
        BT_GATT_PERM_READ, read_es_trigger_setting_cpp,
        NULL, &temp_sensor),

    BT_GATT_CCC(es_ccc_cfg_changed_cpp,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

/* Humidity Sensor */
    BT_GATT_CHARACTERISTIC(BT_UUID_HUMIDITY, BT_GATT_CHRC_READ,
        BT_GATT_PERM_READ,
        read_u16, NULL, &sensor_3.humid_value),
    BT_GATT_CUD(SENSOR_3_NAME, BT_GATT_PERM_READ),
    BT_GATT_DESCRIPTOR(BT_UUID_ES_MEASUREMENT, BT_GATT_PERM_READ,
        read_es_measurement, NULL, &sensor_3.meas),
);

static void ess_simulate(void)
{
  static uint8_t i;
  uint16_t val;

  if (!(i % SENSOR_1_UPDATE_IVAL)) {
    val = 1200 + i;
    update_temperature(NULL, &ess_svc.attrs[2], val, &sensor_1);
  }

  if (!(i % SENSOR_2_UPDATE_IVAL)) {
    val = 1800 + i;
    update_temperature(NULL, &ess_svc.attrs[9], val, &sensor_2);
  }

  if (!(i % SENSOR_3_UPDATE_IVAL)) {
    sensor_3.humid_value = 6233 + (i % 13);
  }

  if (!(i % INT8_MAX)) {
    i = 0U;
  }

  i++;
}

static void bas_notify(void)
{
  uint8_t battery_level = bt_bas_get_battery_level();

  battery_level--;

  if (!battery_level) {
    battery_level = 100U;
  }

  bt_bas_set_battery_level(battery_level);
}

void main(void)
{
  bt::ConnectionManager::begin();

  bt::ConnectionManager::start_adv();

  while (1) {
    k_sleep(K_SECONDS(1));

    /* Temperature simulation */
    if (simulate_temp) {
      ess_simulate();
    }

    /* Battery level simulation */
    bas_notify();
  }
}
