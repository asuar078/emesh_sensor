//
// Created by bigbywolf on 12/27/20.
//

#include "connection_manager.hpp"

#include <host/conn_internal.h>

namespace bt {

  struct bt_conn* ConnectionManager::s_conn = nullptr;
  struct bt_conn_cb ConnectionManager::conn_callbacks{};
  struct bt_conn_auth_cb ConnectionManager::auth_cb_display{};

  bool ConnectionManager::begin()
  {
    unsigned int passkey = 123456;
    bt_passkey_set(passkey);

    int err;

    err = bt_enable(ConnectionManager::bt_dev_ready);
    if (err) {
      printk("Bluetooth init failed (err %d)\n", err);
      return false;
    }

    printk("Bluetooth init successfully\n");

    /* setup callbacks */
    conn_callbacks = {
        .connected = ConnectionManager::connected,
        .disconnected = ConnectionManager::disconnected,
        .identity_resolved = ConnectionManager::identity_resolved,
        .security_changed = ConnectionManager::security_changed,
    };

    bt_conn_cb_register(&conn_callbacks);

    auth_cb_display = {
        .passkey_display = ConnectionManager::auth_passkey_display,
        .passkey_entry = NULL,
        .cancel = ConnectionManager::auth_cancel,
        .pairing_confirm = ConnectionManager::pairing_confirm,
        .pairing_complete = ConnectionManager::pairing_complete,
        .pairing_failed = ConnectionManager::pairing_failed
    };

    bt_conn_auth_cb_register(&auth_cb_display);

    return true;
  }

  void ConnectionManager::bt_dev_ready(int err)
  {
    if (err) {
      printk("Bluetooth init failed (err %d)\n", err);
    }
  }

  bool ConnectionManager::start_adv()
  {
    int err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
      printk("Advertising failed to start (err %d)\n", err);
      return false;
    }

    printk("Advertising successfully started\n");

    return true;
  }

  void ConnectionManager::connected(struct bt_conn* conn, uint8_t err)
  {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
    s_conn = conn;

    if (err) {
      printk("Connection failed (err 0x%02x)\n", err);
      return;
    }

    printk("Connected\n");
    send_con_msg(ConnectionEvent::connected);

    if (bt_conn_set_security(conn, BT_SECURITY_L4)) {
      printk("Failed to set security\n");
    }
  }

  void ConnectionManager::disconnected(struct bt_conn* conn, uint8_t reason)
  {
    s_conn = conn;
    printk("Disconnected (reason 0x%02x)\n", reason);
    send_con_msg(ConnectionEvent::disconnected);
  }

  void ConnectionManager::auth_passkey_display(struct bt_conn* conn, unsigned int passkey)
  {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    printk("Passkey for %s: %06u\n", addr, passkey);
  }

  void ConnectionManager::auth_cancel(struct bt_conn* conn)
  {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    printk("Pairing cancelled: %s\n", addr);
  }

  void ConnectionManager::security_changed(struct bt_conn* conn, bt_security_t level, enum bt_security_err err)
  {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (!err) {
      printk("Security changed: %s level %u\n", addr, level);
    }
    else {
      printk("Security failed: %s level %u err %d\n", addr, level,
          err);
    }
  }

  void ConnectionManager::pairing_confirm(struct bt_conn* conn)
  {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    bt_conn_auth_pairing_confirm(conn);

    printk("Pairing confirmed: %s\n", addr);
  }

  void ConnectionManager::pairing_complete(struct bt_conn* conn, bool bonded)
  {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    printk("Pairing completed: %s, bonded: %d\n", addr, bonded);
  }

  void ConnectionManager::pairing_failed(struct bt_conn* conn, enum bt_security_err reason)
  {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    printk("Pairing failed conn: %s, reason %d\n", addr, reason);
  }

  void ConnectionManager::identity_resolved(struct bt_conn* conn, const bt_addr_le_t* rpa, const bt_addr_le_t* identity)
  {
    char addr_identity[BT_ADDR_LE_STR_LEN];
    char addr_rpa[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(identity, addr_identity, sizeof(addr_identity));
    bt_addr_le_to_str(rpa, addr_rpa, sizeof(addr_rpa));

    printk("Identity resolved %s -> %s\n", addr_rpa, addr_identity);
  }

  bt_conn* ConnectionManager::get_connection()
  {
    return s_conn;
  }


}

