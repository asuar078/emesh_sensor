//
// Created by bigbywolf on 1/14/21.
//

#include "msg_passer.hpp"

namespace bt {

  K_MSGQ_DEFINE(conn_msgq, sizeof(struct bt::ConnectionMsg), bt::CONNECTION_MSG_QUEUE_SIZE , 4);

  void send_con_msg(ConnectionEvent evt)
  {
    ConnectionMsg item{
        .event = evt
    };
    /* send data to consumers */
    while (k_msgq_put(&conn_msgq, &item, K_NO_WAIT) != 0) {
      /* message queue is full: purge old data & try again */
      k_msgq_purge(&conn_msgq);
    }
  }

  ConnectionEvent get_con_msg()
  {
    struct ConnectionMsg ret;
    k_msgq_get(&conn_msgq, &ret, K_FOREVER);
    return ret.event;
  }

}