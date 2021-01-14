/* main.c - Application main entry point */

/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

extern "C" {

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <zephyr.h>
#include <settings/settings.h>

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
#include <connection_manager/connection_handler.hpp>
#include <connection_manager/connection_manager.hpp>
#include <services/ess/environmental_sensing_service.hpp>


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

void temp_ccc_cfg_changed_cb(const struct bt_gatt_attr* attr, uint16_t value)
{
  temp_sensor.get_configuration().set_ccc(value);
}

/* Environmental Sensing Service Declaration */
BT_GATT_SERVICE_DEFINE(ess_svc,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_ESS),

/* Temperature Sensor cpp */
    BT_GATT_CHARACTERISTIC(BT_UUID_TEMPERATURE,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
        BT_GATT_PERM_READ,
        bt::ess::read_value_cb, NULL, &temp_sensor),

    BT_GATT_DESCRIPTOR(BT_UUID_ES_MEASUREMENT, BT_GATT_PERM_READ,
        bt::ess::read_measurement_cb, NULL, &temp_sensor),

    BT_GATT_CUD(temp_sensor.get_name(), BT_GATT_PERM_READ),

    BT_GATT_DESCRIPTOR(BT_UUID_VALID_RANGE, BT_GATT_PERM_READ,
        bt::ess::read_valid_range_cb, NULL, &temp_sensor),

    BT_GATT_DESCRIPTOR(BT_UUID_ES_TRIGGER_SETTING,
        BT_GATT_PERM_READ, bt::ess::read_trigger_setting_cb,
        NULL, &temp_sensor),

    BT_GATT_CCC(temp_ccc_cfg_changed_cb,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

static void bas_notify(void)
{
  uint8_t battery_level = bt_bas_get_battery_level();

  battery_level--;

  if (!battery_level) {
    battery_level = 100U;
  }

  bt_bas_set_battery_level(battery_level);
}


#define DEFAULT_FOO_VAL_VALUE 0

static uint8_t foo_val = DEFAULT_FOO_VAL_VALUE;

static int foo_settings_set(const char* name, size_t len,
    settings_read_cb read_cb, void* cb_arg)
{
  const char* next;
  int rc;

  if (settings_name_steq(name, "bar", &next) && !next) {
    if (len != sizeof(foo_val)) {
      return -EINVAL;
    }

    rc = read_cb(cb_arg, &foo_val, sizeof(foo_val));
    if (rc >= 0) {
      return 0;
    }

    return rc;
  }

  return -ENOENT;
}

struct settings_handler my_conf = {
    .name = "foo",
    .h_set = foo_settings_set
};

//
// using a anonymous namespace for file local variables and functions
//
namespace {

  bt::ConnectionHandler connection_handler{};

} // namespace


void main(void)
{
  using namespace zpp;
  using namespace std::chrono;

  printk("C++ version\n");

  connection_handler.begin();

  connection_handler.start();
//  while (1) {
//    k_sleep(K_SECONDS(1));
//
////    printk("in loop");
//
//    /* update temp sensor */
//    temp_sensor.update_value(NULL, &ess_svc.attrs[2]);
//
//    /* Battery level simulation */
//    bas_notify();
//  }
}
