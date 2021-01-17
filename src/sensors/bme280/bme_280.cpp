//
// Created by bigbywolf on 1/16/21.
//

#include "bme_280.hpp"
#include <logger.hpp>

namespace sensor {

  bool BME280::begin()
  {
    dev_ = device_get_binding("BME280");
    if (dev_ == nullptr) {
      log_msg("No device {} found; did initialization fail?", BME280_LABEL);
      return false;
    }

    log_msg("Found device {}", BME280_LABEL);
    return true;
  }

  void BME280::sample_sensor()
  {
    sensor_sample_fetch(dev_);
    sensor_channel_get(dev_, SENSOR_CHAN_AMBIENT_TEMP, &temp_);
    sensor_channel_get(dev_, SENSOR_CHAN_HUMIDITY, &humidity_);
  }

  std::uint16_t BME280::get_humidity_hecto_pct() const
  {
    return humidity_.val1;
  }

  std::int16_t BME280::get_temperature_hecto_c() const
  {
    return temp_.val1;
  }

   bt::ess::EnvironmentalSensingService& BME280::get_temp_sensor()
  {
    return temp_sensor;
  }
}
