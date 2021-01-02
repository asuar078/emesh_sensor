//
// Created by bigbywolf on 12/28/20.
//

#include "environmental_sensing_service.hpp"
#include <limits>

extern "C" {
#include <sys/byteorder.h>
}

namespace bt::ess {

  EnvironmentalSensingService::EnvironmentalSensingService(const char* name,
      Measurement measurement,
      TriggerSetting trigger_setting,
      Configuration configuration)
      :name_(name),
       measurement_(measurement),
       trigger_setting_(trigger_setting),
       configuration_(configuration),
       valid_range_(ValidRange())
  {
  }

  EnvironmentalSensingService::EnvironmentalSensingService(const char* name,
      Measurement measurement,
      TriggerSetting trigger_setting,
      Configuration configuration,
      ValidRange valid_range
  )
      :name_(name),
       measurement_(measurement),
       trigger_setting_(trigger_setting),
       configuration_(configuration),
       valid_range_(valid_range)
  {
  }

  const char* EnvironmentalSensingService::get_name() const
  {
    return name_;
  }

  const Measurement& EnvironmentalSensingService::get_measurement() const
  {
    return measurement_;
  }

  const TriggerSetting& EnvironmentalSensingService::get_trigger_setting() const
  {
    return trigger_setting_;
  }

  Configuration& EnvironmentalSensingService::get_configuration()
  {
    return configuration_;
  }

  const ValidRange& EnvironmentalSensingService::get_valid_range() const
  {
    return valid_range_;
  }

  int16_t EnvironmentalSensingService::get_value() const
  {
    return value_;
  }

  ssize_t EnvironmentalSensingService::read_value(struct bt_conn* conn, const struct bt_gatt_attr* attr, void* buf,
      uint16_t len, uint16_t offset) const
  {
    uint16_t value = sys_cpu_to_le16(get_value());
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &value,
        sizeof(value));
  }

  bool EnvironmentalSensingService::do_notify(int16_t old_val, int16_t new_val)
  {
    if (configuration_.get_ccc() == Configuration::ClientCharConfig::none) {
      return false;
    }

    int16_t ref_val = trigger_setting_.get_value();

    /* get time since last notify */
    auto last_notify_s = k_uptime_delta(&last_notify_ms_) / 1000;

    switch (trigger_setting_.get_condition()) {
      case TriggerSetting::Condition::trigger_inactive:
        return false;
      case TriggerSetting::Condition::fixed_time_interval:
      case TriggerSetting::Condition::no_less_than_specified_time:
        /* update last notify */
        last_notify_ms_ = k_uptime_get();

        return last_notify_s >= ref_val;
      case TriggerSetting::Condition::value_changed:
        return new_val != old_val;
      case TriggerSetting::Condition::less_than_ref_value:
        return new_val < ref_val;
      case TriggerSetting::Condition::less_or_equal_to_ref_value:
        return new_val <= ref_val;
      case TriggerSetting::Condition::greater_than_ref_value:
        return new_val > ref_val;
      case TriggerSetting::Condition::greater_or_equal_to_ref_value:
        return new_val >= ref_val;
      case TriggerSetting::Condition::equal_to_ref_value:
        return new_val == ref_val;
      case TriggerSetting::Condition::not_equal_to_ref_value:
        return new_val != ref_val;
      default:
        return false;
    }

    return false;
  }

  void EnvironmentalSensingService::update_value(struct bt_conn* conn, const struct bt_gatt_attr* chrc)
  {
    auto new_val = get_new_service_value();
    bool notify = do_notify(value_, new_val);

    /* Update value */
    value_ = new_val;

    /* Trigger notification if conditions are met */
    if (notify) {
      bt_gatt_notify(conn, chrc, &value_, sizeof(value_));
    }
  }

  int16_t EnvironmentalSensingService::get_new_service_value()
  {
    return 0;
  }

  ess::Measurement::Measurement(ess::Measurement::SamplingFunction sampling_function,
      ess::Measurement::Application application_type, uint32_t measurement_period_seconds,
      uint32_t update_interval_seconds, uint8_t measurement_uncertainty_pct)
      :sampling_func_(sampling_function),
       application_(application_type),
       meas_period_s_(measurement_period_seconds),
       update_interval_s_(update_interval_seconds),
       meas_uncertainty_0p5_pct_(measurement_uncertainty_pct * 2)
  {

  }

  ess::Measurement::SamplingFunction ess::Measurement::get_sampling_func() const
  {
    return sampling_func_;
  }

  ess::Measurement::Application ess::Measurement::get_application() const
  {
    return application_;
  }

  uint32_t ess::Measurement::get_meas_period_s() const
  {
    return meas_period_s_;
  }

  uint32_t ess::Measurement::get_update_interval_s() const
  {
    return update_interval_s_;
  }

  uint8_t ess::Measurement::get_meas_uncertainty_0p5pct() const
  {
    return meas_uncertainty_0p5_pct_;
  }

  ssize_t Measurement::read_measurement(struct bt_conn* conn, const struct bt_gatt_attr* attr, void* buf, uint16_t len,
      uint16_t offset) const
  {
    std::array<uint8_t, BUFFER_SIZE> buffer{0};
    buffer.at(0) = 0;
    buffer.at(1) = 0;

    buffer.at(2) = static_cast<uint8_t>(sampling_func_);

    buffer.at(3) = meas_period_s_ & 0xff;
    buffer.at(4) = (meas_period_s_ >> 8) & 0xff;
    buffer.at(5) = (meas_period_s_ >> 16) & 0xff;

    buffer.at(6) = update_interval_s_ & 0xff;
    buffer.at(7) = (update_interval_s_ >> 8) & 0xff;
    buffer.at(8) = (update_interval_s_ >> 16) & 0xff;

    buffer.at(9) = static_cast<uint8_t>(application_);
    buffer.at(10) = meas_uncertainty_0p5_pct_;

    return bt_gatt_attr_read(conn, attr, buf, len, offset, buffer.data(),
        buffer.size());
  }

  TriggerSetting::TriggerSetting(TriggerSetting::Condition condition)
      :condition_(condition),
       value_(0)
  {
  }

  TriggerSetting::TriggerSetting(TriggerSetting::Condition condition, uint32_t value)
      :condition_(condition),
       value_(value)
  {
  }

  uint32_t TriggerSetting::get_value() const
  {
    return value_;
  }

  void TriggerSetting::set_value(uint32_t value)
  {
    value_ = value;
  }

  TriggerSetting::Condition TriggerSetting::get_condition() const
  {
    return condition_;
  }

  void TriggerSetting::set_condition(TriggerSetting::Condition condition)
  {
    condition_ = condition;
  }

  ssize_t
  TriggerSetting::read_trigger_setting(struct bt_conn* conn, const struct bt_gatt_attr* attr, void* buf, uint16_t len,
      uint16_t offset) const
  {
    size_t buf_size = 0;
    std::array<uint8_t, BUFFER_SIZE> buffer{0};
    switch (condition_) {
      case Condition::trigger_inactive:
      case Condition::value_changed:
        buf_size = 1;
        buffer.at(0) = static_cast<uint8_t>(condition_);
      case Condition::fixed_time_interval:
      case Condition::no_less_than_specified_time:
        buf_size = 4;
        buffer.at(0) = static_cast<uint8_t>(condition_);
        buffer.at(1) = value_ & 0xff;
        buffer.at(2) = (value_ >> 8) & 0xff;
        buffer.at(3) = (value_ >> 16) & 0xff;
      case Condition::less_than_ref_value:
      case Condition::less_or_equal_to_ref_value:
      case Condition::greater_than_ref_value:
      case Condition::greater_or_equal_to_ref_value:
      case Condition::equal_to_ref_value:
      case Condition::not_equal_to_ref_value:
      default:
        buf_size = 3;
        buffer.at(0) = static_cast<uint8_t>(condition_);
        buffer.at(1) = value_ & 0xff;
        buffer.at(2) = (value_ >> 8) & 0xff;
    }

    return bt_gatt_attr_read(conn, attr, buf, len, offset,
        buffer.data(), buf_size);
  }

  Configuration::Configuration(Configuration::ClientCharConfig ccc)
      :ccc_(ccc)
  {

  }

  Configuration::ClientCharConfig Configuration::get_ccc() const
  {
    return ccc_;
  }

  void Configuration::set_ccc(Configuration::ClientCharConfig ccc)
  {
    ccc_ = ccc;
  }

  const std::array<uint8_t, Configuration::BUFFER_SIZE>& Configuration::get_buffer()
  {
    buffer.at(0) = static_cast<uint8_t>(ccc_);
    return buffer;
  }

  ValidRange::ValidRange()
      :lower_limit_(std::numeric_limits<int16_t>::min()),
       upper_limit_(std::numeric_limits<int16_t>::max())
  {
  }

  ValidRange::ValidRange(int16_t lower_limit, int16_t upper_limit)
      :lower_limit_(lower_limit),
       upper_limit_(upper_limit)
  {
  }

  int16_t ValidRange::get_lower_limit() const
  {
    return lower_limit_;
  }

  int16_t ValidRange::get_upper_limit() const
  {
    return upper_limit_;
  }

  ssize_t ValidRange::read_valid_range(struct bt_conn* conn, const struct bt_gatt_attr* attr, void* buf, uint16_t len,
      uint16_t offset) const
  {
    std::array<uint8_t, BUFFER_SIZE> buffer{0};

    buffer.at(0) = lower_limit_ & 0xff;
    buffer.at(1) = (lower_limit_ >> 8) & 0xff;

    buffer.at(2) = upper_limit_ & 0xff;
    buffer.at(3) = (upper_limit_ >> 8) & 0xff;

    return bt_gatt_attr_read(conn, attr, buf, len, offset, buffer.data(),
        BUFFER_SIZE);
  }

}