//
// Created by bigbywolf on 12/23/20.
//

#ifndef EMESH_SENSOR_HUMIDITY_SENSOR_HPP
#define EMESH_SENSOR_HUMIDITY_SENSOR_HPP

#include <cstdint>

namespace sensor {
  class HumiditySensor {
    public:
      /**
       * Get humidity sensor value in hecto percent humidity.
       * Basically humidity_pct * 100
       * @return a hecto percent humidity between 0 - 10,000
       */
      [[nodiscard]] std::uint16_t get_humidity_hecto_pct() const;
  };
}

#endif //EMESH_SENSOR_HUMIDITY_SENSOR_HPP
