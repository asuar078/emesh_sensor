//
// Created by bigbywolf on 1/16/21.
//

#ifndef EMESH_SENSOR_BME_280_HPP
#define EMESH_SENSOR_BME_280_HPP

extern "C" {
#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/sensor.h>
};

//#define BME280 DT_INST(0, bosch_bme280)
//#define BME280_LABEL DT_LABEL(BME280)
//
//#if !DT_NODE_HAS_STATUS(BME280, okay)
//#error Your devicetree has no enabled nodes with compatible "bosch,bme280"
//#endif

#include <interface/humidity/humidity_sensor.hpp>
#include <interface/temperature/temperature_sensor.hpp>

#include <services/ess/environmental_sensing_service.hpp>

namespace sensor {

  class BME280 {
    public:

      BME280() = default;

      bool begin();

      void sample_sensor();

    private:
      const struct device *dev_{};
      struct sensor_value temp_{}, humidity_{};
  };
}

#endif //EMESH_SENSOR_BME_280_HPP
