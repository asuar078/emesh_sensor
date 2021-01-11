//
// Created by bigbywolf on 1/10/21.
//

#include "connection_handler.hpp"
#include <cstddef>

extern "C" {
#include <settings/settings.h>
#include <zephyr/types.h>
}

namespace bt {

  auto adv_timer = zpp::make_timer(ConnectionHandler::adv_timer_expired);

  static int settings_runtime_load(void)
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

  ConnectionHandler::ConnectionHandler()
  {
  }

  void ConnectionHandler::start()
  {
    s_thread = zpp::thread(
        s_thread_tcb, s_thread_attr, [this](int) {
          this->run();
        }, 0);
  }

  void ConnectionHandler::run()
  {

    bt::ConnectionManager::begin();

    settings_subsys_init();
    settings_load();
    settings_runtime_load();

    bt::ConnectionManager::start_adv();

    adv_timer.start(ADV_TIMEOUT_S);

    while (running) {

      auto ret = ConnectionManager::get_conn_fifo().pop_front();
      zpp::print("connection event {}\n", static_cast<int8_t>(ret->event));

      switch (ret->event) {
        case ConnectionEvent::none:
          break;
        case ConnectionEvent::connected:
          adv_timer.stop();
          break;
        case ConnectionEvent::disconnected:
          adv_timer.start(ADV_TIMEOUT_S);
          break;
        case ConnectionEvent::adv_timed_out:
          zpp::print("adv timer expired");
          bt_le_adv_stop();
          break;
        case ConnectionEvent::notify_central:
          break;
      }

//      zpp::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }

  void ConnectionHandler::adv_timer_expired(zpp::timer_base* t)
  {
    ConnectionItem item {
      .event = ConnectionEvent::adv_timed_out
    };
    ConnectionManager::get_conn_fifo().push_back(&item);
  }

}
