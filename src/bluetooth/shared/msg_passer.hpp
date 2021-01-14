//
// Created by bigbywolf on 1/14/21.
//

#ifndef EMESH_SENSOR_MSG_PASSER_HPP
#define EMESH_SENSOR_MSG_PASSER_HPP

#include <zpp.hpp>
#include <kernel.h>
#include <syscall.h>


namespace bt {

  enum class ConnectionEvent : uint8_t {
      none = 0,
      connected,
      disconnected,
      adv_timed_out,
      notify_central,
      adv_btn_short_press,
      adv_btn_long_press
  };

  struct ConnectionMsg {
      ConnectionEvent event = ConnectionEvent::none;
  };

  static constexpr const size_t CONNECTION_MSG_QUEUE_SIZE = 25;

  void con_msg_init();

  void send_con_msg(ConnectionEvent evt);

  ConnectionEvent get_con_msg();
}

#endif //EMESH_SENSOR_MSG_PASSER_HPP
