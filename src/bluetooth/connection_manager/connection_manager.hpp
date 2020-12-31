//
// Created by bigbywolf on 12/27/20.
//

#ifndef EMESH_SENSOR_CONNECTION_MANAGER_HPP
#define EMESH_SENSOR_CONNECTION_MANAGER_HPP

extern "C" {
#include <stdbool.h>
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <zephyr.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/bas.h>
}

#include <bounce.hpp>

namespace bt {

  static const struct bt_data ad[] = {
      BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
      BT_DATA_BYTES(BT_DATA_GAP_APPEARANCE, 0x00, 0x03),
      BT_DATA_BYTES(BT_DATA_UUID16_ALL,
          BT_UUID_16_ENCODE(BT_UUID_ESS_VAL),
          BT_UUID_16_ENCODE(BT_UUID_BAS_VAL)),
  };

  class ConnectionManager {
    public:
      static bool begin();

      static bool start_adv();

    private:
      static struct bt_conn* s_conn;
      static struct bt_conn_cb conn_callbacks;
      static struct bt_conn_auth_cb auth_cb_display;

      static void bt_dev_ready(int err);

      static void connected(struct bt_conn* conn, uint8_t err);

      static void disconnected(struct bt_conn* conn, uint8_t reason);

      static void auth_passkey_display(struct bt_conn* conn, unsigned int passkey);

      static void auth_cancel(struct bt_conn* conn);
  };

}

#endif //EMESH_SENSOR_CONNECTION_MANAGER_HPP
