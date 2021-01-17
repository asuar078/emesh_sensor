//
// Created by bigbywolf on 12/23/20.
//

#include "temperature_sensor.hpp"

namespace sensor {
  std::int16_t TemperatureSensor::get_temperature_hecto_c() const
  {
    return 0;
  }

  int16_t TemperatureSensor::read_sensor()
  {
    return get_temperature_hecto_c();
  }

  bt::ess::EnvironmentalSensingService& TemperatureSensor::get_temp_sensor()
  {
    return temp_sensor;
  }

}
