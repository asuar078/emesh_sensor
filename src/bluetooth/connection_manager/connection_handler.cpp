//
// Created by bigbywolf on 1/10/21.
//

#include "connection_handler.hpp"
#include <cstddef>
#include <adv_btn/advertise_button.hpp>

extern "C" {
#include <settings/settings.h>
#include <zephyr/types.h>
}

namespace bt {

  static auto adv_timer = zpp::make_timer(ConnectionHandler::adv_timer_expired);

  static int settings_runtime_load()
  {
#if defined(CONFIG_BT_DIS_SETTINGS)
    settings_runtime_set("bt/dis/model",
           CONFIG_BT_DIS_MODEL,
           sizeof(CONFIG_BT_DIS_MODEL));
  settings_runtime_set("bt/dis/manuf",
           CONFIG_BT_DIS_MANUF,
           sizeof(CONFIG_BT_DIS_MANUF));
#if defined(CONFIG_BT_DIS_SERIAL_NUMBER)
  settings_runtime_set("bt/dis/serial",
           CONFIG_BT_DIS_SERIAL_NUMBER_STR,
           sizeof(CONFIG_BT_DIS_SERIAL_NUMBER_STR));
#endif
#if defined(CONFIG_BT_DIS_SW_REV)
  settings_runtime_set("bt/dis/sw",
           CONFIG_BT_DIS_SW_REV_STR,
           sizeof(CONFIG_BT_DIS_SW_REV_STR));
#endif
#if defined(CONFIG_BT_DIS_FW_REV)
  settings_runtime_set("bt/dis/fw",
           CONFIG_BT_DIS_FW_REV_STR,
           sizeof(CONFIG_BT_DIS_FW_REV_STR));
#endif
#if defined(CONFIG_BT_DIS_HW_REV)
  settings_runtime_set("bt/dis/hw",
           CONFIG_BT_DIS_HW_REV_STR,
           sizeof(CONFIG_BT_DIS_HW_REV_STR));
#endif
#endif
    return 0;
  }

  ConnectionHandler::ConnectionHandler() = default;

  bool ConnectionHandler::begin()
  {
    return adv_btn.begin() && bt::ConnectionManager::begin();
  }

  void ConnectionHandler::start()
  {
    adv_btn.start();

    s_thread = zpp::thread(
        s_thread_tcb, s_thread_attr, [this](int) {
          this->run();
        }, 0);
  }

  void ConnectionHandler::run()
  {
    ConnectionEvent ret;
    settings_subsys_init();
    settings_load();
    settings_runtime_load();

    bt::ConnectionManager::start_adv();

    adv_timer.start(ADV_TIMEOUT_S);

    while (running) {

      ret = get_con_msg();
      zpp::print("event received: {}\n", static_cast<int8_t>(ret));

      switch (ret) {
        case ConnectionEvent::none:
          break;
        case ConnectionEvent::connected:
          zpp::print("connected, stopping adv timer\n");
          adv_timer.stop();
          break;
        case ConnectionEvent::disconnected:
          zpp::print("restarting adv timer\n");
          adv_timer.start(ADV_TIMEOUT_S);
          break;
        case ConnectionEvent::adv_timed_out:
          zpp::print("adv timer expired\n");
          bt_le_adv_stop();
          break;
        case ConnectionEvent::notify_central:
          break;
        case ConnectionEvent::adv_btn_short_press:
          zpp::print("short press\n");
          // show battery life or other information
          break;
        case ConnectionEvent::adv_btn_long_press:
          zpp::print("long press\n");
          if (adv_timer.status() > 0) {
            zpp::print("restarting adv\n");
            bt::ConnectionManager::start_adv();
            adv_timer.start(ADV_TIMEOUT_S);
          }
          else {
            zpp::print("adv still in effect\n");
          }
          break;
      }

//      zpp::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }

  void ConnectionHandler::adv_timer_expired(zpp::timer_base* t)
  {
    send_con_msg(ConnectionEvent::adv_timed_out);
  }

}

