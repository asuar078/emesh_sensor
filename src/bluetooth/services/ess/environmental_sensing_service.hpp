//
// Created by bigbywolf on 12/28/20.
//

#ifndef EMESH_SENSOR_ENVIRONMENTAL_SENSING_SERVICE_HPP
#define EMESH_SENSOR_ENVIRONMENTAL_SENSING_SERVICE_HPP

extern "C" {
#include <bluetooth/gatt.h>
};

#include <cstdint>
#include <array>

namespace bt::ess {

  class Measurement {
    public:
      enum class SamplingFunction : uint8_t {
          unspecified = 0x00,
          instantaneous = 0x01,
          arithmetic_mean = 0x02,
          rms = 0x03,
          maximum = 0x04,
          minimum = 0x05,
          accumulated = 0x06,
          count = 0x07
      };

      enum class Application : uint8_t {
          unspecified = 0x00,
          air = 0x01,
          water = 0x02,
          barometric = 0x03,
          soil = 0x04,
          infrared = 0x05,
          map_database = 0x06,
          barometric_elevation = 0x07,
          gps_only_elevation = 0x08,
          gps_map_data_base_elevation = 0x09,
          vertical_datum_elevation = 0x0A,
          onshore = 0x0B,
          on_board_vessel_or_vehicle = 0x0C,
          front = 0x0D,
          back_rear = 0x0E,
          upper = 0x0F,
          lower = 0x10,
          primary = 0x11,
          secondary = 0x12,
          outdoor = 0x13,
          indoor = 0x14,
          top = 0x15,
          bottom = 0x16,
          main = 0x17,
          backup = 0x18,
          auxiliary = 0x19,
          supplementary = 0x1A,
          inside = 0x1B,
          outside = 0x1C,
          left = 0x1D,
          right = 0x1E,
          internal = 0x1F,
          external = 0x20,
          solar = 0x21,
      };

      static constexpr size_t BUFFER_SIZE = 11;

      Measurement(SamplingFunction sampling_function,
          Application application_type,
          uint32_t measurement_period_seconds,
          uint32_t update_interval_seconds,
          uint8_t measurement_uncertainty_pct);

      SamplingFunction get_sampling_func() const;

      Application get_application() const;

      uint32_t get_meas_period_s() const;

      uint32_t get_update_interval_s() const;

      uint8_t get_meas_uncertainty_0p5pct() const;

      ssize_t read_measurement(struct bt_conn* conn,
          const struct bt_gatt_attr* attr, void* buf,
          uint16_t len, uint16_t offset) const;

    private:
      SamplingFunction sampling_func_;
      Application application_;
      uint32_t meas_period_s_; /* seconds */
      uint32_t update_interval_s_; /* seconds */
      uint8_t meas_uncertainty_0p5_pct_; /* 0.5 percent */
  };

  class TriggerSetting {
    public:
      enum class Condition : uint8_t {
          trigger_inactive = 0x00,
          fixed_time_interval = 0x01,
          no_less_than_specified_time = 0x02,
          value_changed = 0x03,
          less_than_ref_value = 0x04,
          less_or_equal_to_ref_value = 0x05,
          greater_than_ref_value = 0x06,
          greater_or_equal_to_ref_value = 0x07,
          equal_to_ref_value = 0x08,
          not_equal_to_ref_value = 0x09,
      };

      static constexpr size_t BUFFER_SIZE = 4;

      explicit TriggerSetting(Condition condition);

      TriggerSetting(Condition condition, uint32_t value);

      Condition get_condition() const;

      void set_condition(Condition condition);

      uint32_t get_value() const;

      void set_value(uint32_t value);

      ssize_t read_trigger_setting(struct bt_conn* conn,
          const struct bt_gatt_attr* attr,
          void* buf, uint16_t len,
          uint16_t offset) const;

    private:
      Condition condition_;
      uint32_t value_;
  };

  class Configuration {
    public:
      enum class ClientCharConfig : uint8_t {
          none = 0,
          notify = BT_GATT_CCC_NOTIFY,
          indicate = BT_GATT_CCC_INDICATE
      };

      static constexpr size_t BUFFER_SIZE = 1;

      explicit Configuration(ClientCharConfig ccc);

      ClientCharConfig get_ccc() const;

      void set_ccc(ClientCharConfig ccc);

      void set_ccc(uint16_t ccc);

      [[nodiscard]] const std::array<uint8_t, BUFFER_SIZE>& get_buffer();

    private:
      ClientCharConfig ccc_;

      std::array<uint8_t, BUFFER_SIZE> buffer{0};
  };

  class ValidRange {
    public:
      static constexpr size_t BUFFER_SIZE = 4;

      ValidRange();

      ValidRange(int16_t lower_limit, int16_t upper_limit);

      int16_t get_lower_limit() const;

      int16_t get_upper_limit() const;

      ssize_t read_valid_range(struct bt_conn* conn,
          const struct bt_gatt_attr* attr, void* buf,
          uint16_t len, uint16_t offset) const;

    private:
      int16_t lower_limit_;
      int16_t upper_limit_;

  };

  class EnvironmentalSensingService {
    public:
      static constexpr size_t BUFFER_SIZE = 2;

      EnvironmentalSensingService(const char* name,
          Measurement measurement,
          TriggerSetting trigger_setting,
          Configuration configuration
      );

      EnvironmentalSensingService(const char* name,
          Measurement measurement,
          TriggerSetting trigger_setting,
          Configuration configuration,
          ValidRange valid_range
      );

      const char* get_name() const;

      const Measurement& get_measurement() const;

      const TriggerSetting& get_trigger_setting() const;

      Configuration& get_configuration();

      const ValidRange& get_valid_range() const;

      virtual int16_t get_value() const;

      ssize_t read_value(struct bt_conn* conn,
          const struct bt_gatt_attr* attr,
          void* buf, uint16_t len, uint16_t offset);

      void update_value(struct bt_conn* conn, const struct bt_gatt_attr* chrc);

    protected:
      const char* name_;
      const Measurement measurement_;
      TriggerSetting trigger_setting_;
      Configuration configuration_;
      const ValidRange valid_range_;

      int16_t value_ = 0;
      int64_t last_update_ms_ = 0;
      int64_t last_notify_ms_ = 0;

      bool do_notify(int16_t old_val, int16_t new_val);

      virtual int16_t read_sensor();
  };

  ssize_t read_value_cb(struct bt_conn* conn, const struct bt_gatt_attr* attr,
      void* buf, uint16_t len, uint16_t offset);

  ssize_t read_measurement_cb(struct bt_conn* conn,
      const struct bt_gatt_attr* attr, void* buf,
      uint16_t len, uint16_t offset);

  void ccc_cfg_changed_cb(const struct bt_gatt_attr* attr, uint16_t value);

  ssize_t read_valid_range_cb(struct bt_conn* conn,
      const struct bt_gatt_attr* attr, void* buf,
      uint16_t len, uint16_t offset);

  ssize_t read_trigger_setting_cb(struct bt_conn* conn,
      const struct bt_gatt_attr* attr,
      void* buf, uint16_t len,
      uint16_t offset);
}

#endif //EMESH_SENSOR_ENVIRONMENTAL_SENSING_SERVICE_HPP
