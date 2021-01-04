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

static int settings_runtime_load(void)
{
#if defined(CONFIG_BT_DIS_SETTINGS)
  settings_runtime_set("bt/dis/model",
			     CONFIG_BT_DIS_MODEL,
			     sizeof(CONFIG_BT_DIS_MODEL));
	settings_runtime_set("bt/dis/manuf",
			     CONFIG_BT_DIS_MANUF,
			     sizeof(CONFIG_BT_DIS_MANUF));
#if defined(CONFIG_BT_DIS_SERIAL_NUMBER)
	settings_runtime_set("bt/dis/serial",
			     CONFIG_BT_DIS_SERIAL_NUMBER_STR,
			     sizeof(CONFIG_BT_DIS_SERIAL_NUMBER_STR));
#endif
#if defined(CONFIG_BT_DIS_SW_REV)
	settings_runtime_set("bt/dis/sw",
			     CONFIG_BT_DIS_SW_REV_STR,
			     sizeof(CONFIG_BT_DIS_SW_REV_STR));
#endif
#if defined(CONFIG_BT_DIS_FW_REV)
	settings_runtime_set("bt/dis/fw",
			     CONFIG_BT_DIS_FW_REV_STR,
			     sizeof(CONFIG_BT_DIS_FW_REV_STR));
#endif
#if defined(CONFIG_BT_DIS_HW_REV)
	settings_runtime_set("bt/dis/hw",
			     CONFIG_BT_DIS_HW_REV_STR,
			     sizeof(CONFIG_BT_DIS_HW_REV_STR));
#endif
#endif
  return 0;
}

void main(void)
{
  printk("C++ version\n");

  bt::ConnectionManager::begin();

  if (IS_ENABLED(CONFIG_SETTINGS)) {
    settings_load();
  }

  settings_runtime_load();

  bt::ConnectionManager::start_adv();

  while (1) {
    k_sleep(K_SECONDS(1));

    /* update temp sensor */
    temp_sensor.update_value(NULL, &ess_svc.attrs[2]);

    /* Battery level simulation */
    bas_notify();
  }
}
