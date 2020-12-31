//
// Created by bigbywolf on 12/23/20.
//

#ifndef EMESH_SENSOR_TEMPERATURE_SENSOR_HPP
#define EMESH_SENSOR_TEMPERATURE_SENSOR_HPP

#include <cstdint>

namespace sensor {
  class TemperatureSensor {
    public:
      /**
       * Get temperature sensor value in hecto degrees Celsius.
       * Basically temp_c * 100
       * @return a hecto degrees Celsius.
       */
      [[nodiscard]] std::int16_t get_temperature_hecto_c() const;
  };
}

#endif //EMESH_SENSOR_TEMPERATURE_SENSOR_HPP
