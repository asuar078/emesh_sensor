//
// Created by bigbywolf on 1/10/21.
//

#ifndef EMESH_SENSOR_CONNECTION_HANDLER_HPP
#define EMESH_SENSOR_CONNECTION_HANDLER_HPP

#include "connection_manager.hpp"
#include <zpp.hpp>
#include <chrono>

namespace bt {

  class ConnectionHandler {
    public:

      static constexpr const auto ADV_TIMEOUT_S = std::chrono::seconds(5);
//      #define ADV_TIMEOUT_S K_SECONDS(5)

      ConnectionHandler();

      void start();
//      {
//        s_thread = zpp::thread(
//            s_thread_tcb, s_thread_attr, [this](int) {
//              this->run();
//            }, 0);
//      }

      void run();
//      {
//        while (true) {
//          zpp::print("Second thread test\n");
//          zpp::this_thread::sleep_for(std::chrono::seconds(1));
//        }
//      }

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

  };
}

#endif //EMESH_SENSOR_CONNECTION_HANDLER_HPP
