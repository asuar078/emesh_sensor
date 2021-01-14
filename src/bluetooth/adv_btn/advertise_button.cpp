//
// Created by bigbywolf on 1/13/21.
//

#include "advertise_button.hpp"
#include <connection_manager/connection_manager.hpp>

namespace bt {

  AdvertiseButton::AdvertiseButton()
      :btn_callBack(etl::delegate<void(size_t)>::create<AdvertiseButton, &AdvertiseButton::button_pressed_delegate>(*this))
  {
    k_sem_init(&btn_sem, 0, 1);
    /* register interrupt handler and configure pin */
    get_adv_btn_int_vect_instance().register_delegate(AdvBtnVectorId::button_gpio_int,
        btn_callBack);
  }

  bool AdvertiseButton::begin()
  {
    adv_button = device_get_binding(SW0_GPIO_LABEL);
    if (adv_button == nullptr) {
      printk("Error: didn't find %s device\n", SW0_GPIO_LABEL);
      return false;
    }

    enable_interrupt();

    gpio_init_callback(&adv_button_cb_data, button_gpio_int_handler, BIT(SW0_GPIO_PIN));
    gpio_add_callback(adv_button, &adv_button_cb_data);
    printk("Set up button at %s pin %d\n", SW0_GPIO_LABEL, SW0_GPIO_PIN);

    return true;
  }

  void AdvertiseButton::button_pressed_delegate(size_t)
  {
    /* disable call back until processed */
    gpio_remove_callback(adv_button, &adv_button_cb_data);
    /* notify thread that button was pressed */
    k_sem_give(&btn_sem);
  }

  bool AdvertiseButton::enable_interrupt()
  {
    int ret = gpio_pin_configure(adv_button, SW0_GPIO_PIN, SW0_EN_GPIO_FLAGS);
    if (ret != 0) {
      printk("Error %d: failed to configure %s pin %d\n",
          ret, SW0_GPIO_LABEL, SW0_GPIO_PIN);
      return false;
    }
    return true;
  }

  bool AdvertiseButton::disable_interrupt()
  {
    int ret = gpio_pin_configure(adv_button, SW0_GPIO_PIN, SW0_DIS_GPIO_FLAGS);
    if (ret != 0) {
      printk("Error %d: failed to configure %s pin %d\n",
          ret, SW0_GPIO_LABEL, SW0_GPIO_PIN);
      return false;
    }
    return true;
  }

  void AdvertiseButton::start()
  {
    s_thread = zpp::thread(
        s_thread_tcb, s_thread_attr, [this](int) {
          this->run();
        }, 0);
  }

  void AdvertiseButton::run()
  {
    int sem_ret = 0;

    while (running) {

      sem_ret = k_sem_take(&btn_sem, K_MSEC(500));
      if (sem_ret != 0) {
        continue;
      }

      /* wait till signal stabilizes */
      zpp::this_thread::sleep_for(DEBOUNCE_TIME_MS);

      bool long_hold = true;
      /* loop till not pressed or hold time passes */
      for (int samples = 0; samples < NUM_OF_HOLD_SAMPLES; ++samples) {
        /* check twice for not pressed */
        if (get_level() != PRESSED_LVL) {
          zpp::this_thread::sleep_for(SAMPLE_TIME);
          if (get_level() != PRESSED_LVL) {
            long_hold = false;
            break;
          }
        }
        zpp::this_thread::sleep_for(SAMPLE_TIME);
      }

      if (long_hold) {
        send_con_msg(ConnectionEvent::adv_btn_long_press);
      }
      else {
        send_con_msg(ConnectionEvent::adv_btn_short_press);
      }

      /* wait for the btn to not be pressed */
      while (get_level() == PRESSED_LVL) {
        zpp::this_thread::sleep_for(std::chrono::milliseconds(100));
      }

      /* re-enable interrupt callback */
      gpio_add_callback(adv_button, &adv_button_cb_data);
    }
  }

  bool AdvertiseButton::get_level()
  {
    return gpio_pin_get(adv_button, SW0_GPIO_PIN);
  }

}

AdvBtnInterruptVectors& get_adv_btn_int_vect_instance()
{
  static AdvBtnInterruptVectors interruptVectors;
  return interruptVectors;
}

void button_gpio_int_handler(const struct device* dev, struct gpio_callback* cb, uint32_t pins)
{
  get_adv_btn_int_vect_instance().call<AdvBtnVectorId::button_gpio_int>();
}

