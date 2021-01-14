//
// Created by bigbywolf on 1/13/21.
//

#ifndef EMESH_SENSOR_ADVERTISE_BUTTON_HPP
#define EMESH_SENSOR_ADVERTISE_BUTTON_HPP

extern "C" {
#include <device.h>
#include <drivers/gpio.h>
#include <sys/util.h>
};

#include <shared/msg_passer.hpp>
#include <etl/delegate.h>
#include <etl/delegate_service.h>

#include <chrono>
#include <cinttypes>

/*
 * Get button configuration from the devicetree sw0 alias.
 *
 * At least a GPIO device and pin number must be provided. The 'flags'
 * cell is optional.
 */
#define SW0_NODE  DT_ALIAS(sw0)
//#if DT_NODE_HAS_STATUS(SW0_NODE, okay)
#define SW0_GPIO_LABEL	DT_GPIO_LABEL(SW0_NODE, gpios)
#define SW0_GPIO_PIN	DT_GPIO_PIN(SW0_NODE, gpios)
#define SW0_EN_GPIO_FLAGS	(GPIO_INPUT | DT_GPIO_FLAGS(SW0_NODE, gpios) | GPIO_INT_ENABLE | GPIO_INT_EDGE_FALLING | GPIO_PULL_UP)
#define SW0_DIS_GPIO_FLAGS	(GPIO_INPUT | DT_GPIO_FLAGS(SW0_NODE, gpios) | GPIO_INT_DISABLE | GPIO_PULL_UP)

#if !DT_NODE_HAS_STATUS(SW0_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif

enum AdvBtnVectorId {
  button_gpio_int = 1,
  adv_btn_vector_id_end,
  adv_btn_vector_id_offset = button_gpio_int,
  adv_btn_vector_id_range = adv_btn_vector_id_end - adv_btn_vector_id_offset
};

using AdvBtnInterruptVectors = etl::delegate_service<adv_btn_vector_id_range, adv_btn_vector_id_offset>;

/* Ensure that the callback service is initialised before use */
static AdvBtnInterruptVectors& get_adv_btn_int_vect_instance();

extern "C" {
void button_gpio_int_handler(const struct device* dev, struct gpio_callback* cb, uint32_t pins);
}

namespace bt {

  class AdvertiseButton {
    public:
      /* The sample time in ms. */
      static constexpr const auto SAMPLE_TIME = std::chrono::milliseconds(100);

      static constexpr const auto DEBOUNCE_TIME_MS = std::chrono::milliseconds(300);
      static constexpr const auto LONG_PRESS_HOLD_TIME_MS = std::chrono::milliseconds(1000);
      static constexpr const auto NUM_OF_DEBOUNCE_SAMPLES = DEBOUNCE_TIME_MS / SAMPLE_TIME;
      static constexpr const auto NUM_OF_HOLD_SAMPLES =
          (LONG_PRESS_HOLD_TIME_MS / SAMPLE_TIME) - NUM_OF_DEBOUNCE_SAMPLES;

      static constexpr const bool PRESSED_LVL = true;

      AdvertiseButton();

      bool begin();

      void start();

      void run();

      bool get_level();

    private:
      etl::delegate<void(size_t)> btn_callBack;
      struct k_sem btn_sem{};
      const struct device* adv_button = nullptr;
      struct gpio_callback adv_button_cb_data{};

      bool running = true;
      zpp::thread s_thread;
      zpp::thread_data<1024> s_thread_tcb;
      const zpp::thread_attr s_thread_attr{
          zpp::thread_prio::preempt(0),
          zpp::thread_inherit_perms::no,
          zpp::thread_suspend::no
      };

      void button_pressed_delegate(size_t);

      bool enable_interrupt();

      bool disable_interrupt();

  };
}

#endif //EMESH_SENSOR_ADVERTISE_BUTTON_HPP
