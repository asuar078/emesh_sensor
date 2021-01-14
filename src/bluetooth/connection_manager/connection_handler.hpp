//
// Created by bigbywolf on 1/10/21.
//

#ifndef EMESH_SENSOR_CONNECTION_HANDLER_HPP
#define EMESH_SENSOR_CONNECTION_HANDLER_HPP

extern "C" {
#include <device.h>
#include <drivers/gpio.h>
#include <sys/util.h>
};

#include "connection_manager.hpp"
#include "adv_btn/advertise_button.hpp"

#include <chrono>

namespace bt {

  class ConnectionHandler {
    public:

      static constexpr const auto ADV_TIMEOUT_S = std::chrono::seconds(30);

      ConnectionHandler();

      bool begin();

      void start();

      void run();

      static void adv_timer_expired(zpp::timer_base* t);


    private:
      bool running = true;
      zpp::thread s_thread;
      zpp::thread_data<1024> s_thread_tcb;
      const zpp::thread_attr s_thread_attr{
          zpp::thread_prio::preempt(0),
          zpp::thread_inherit_perms::no,
          zpp::thread_suspend::no
      };

      ConnectionManager conn_man;
      AdvertiseButton adv_btn;
  };
}

#endif //EMESH_SENSOR_CONNECTION_HANDLER_HPP
