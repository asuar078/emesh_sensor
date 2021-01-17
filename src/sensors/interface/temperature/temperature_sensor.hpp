//
// Created by bigbywolf on 12/23/20.
//

#ifndef EMESH_SENSOR_TEMPERATURE_SENSOR_HPP
#define EMESH_SENSOR_TEMPERATURE_SENSOR_HPP

#include <services/ess/environmental_sensing_service.hpp>
#include <cstdint>

namespace sensor {
  class TemperatureSensor : public bt::ess::EnvironmentalSensingService {
    public:
      /**
       * Get temperature sensor value in hecto degrees Celsius.
       * Basically temp_c * 100
       * @return a hecto degrees Celsius.
       */
      [[nodiscard]] virtual std::int16_t get_temperature_hecto_c() const;

      bt::ess::EnvironmentalSensingService& get_temp_sensor() ;

    protected:

      bt::ess::EnvironmentalSensingService temp_sensor {
          "Temperature Sensor",
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

      int16_t read_sensor() override;
  };
}

#endif //EMESH_SENSOR_TEMPERATURE_SENSOR_HPP
