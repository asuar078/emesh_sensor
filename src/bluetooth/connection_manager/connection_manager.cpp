//
// Created by bigbywolf on 12/27/20.
//

#include "connection_manager.hpp"

#include <host/conn_internal.h>
#include <settings/settings.h>

namespace bt {

  struct bt_conn* ConnectionManager::s_conn = nullptr;
  struct bt_conn_cb ConnectionManager::conn_callbacks{};
  struct bt_conn_auth_cb ConnectionManager::auth_cb_display{};

  bool ConnectionManager::begin()
  {
    int err;

    err = bt_enable(ConnectionManager::bt_dev_ready);
    if (err) {
      printk("Bluetooth init failed (err %d)\n", err);
      return false;
    }

    printk("Bluetooth init successfully\n");
    return true;
  }

  void ConnectionManager::bt_dev_ready(int err)
  {
    if (IS_ENABLED(CONFIG_SETTINGS)) {
      settings_load();
    }

    if (err) {
      printk("Bluetooth init failed (err %d)\n", err);
    }
  }

  bool ConnectionManager::start_adv()
  {
    int err;

//    bt_le_set_auto_conn();
    err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
      printk("Advertising failed to start (err %d)\n", err);
      return false;
    }

    printk("Advertising successfully started\n");

    conn_callbacks = {
        .connected = ConnectionManager::connected,
        .disconnected = ConnectionManager::disconnected
    };

    bt_conn_cb_register(&conn_callbacks);

    auth_cb_display = {
        .passkey_display = ConnectionManager::auth_passkey_display,
        .passkey_entry = NULL,
        .cancel = ConnectionManager::auth_cancel,
    };

    bt_conn_auth_cb_register(&auth_cb_display);

    return true;
  }

  void ConnectionManager::connected(struct bt_conn* conn, uint8_t err)
  {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
    s_conn = conn;



    if (err) {
      printk("Connection failed (err 0x%02x)\n", err);
    }
    else {
      printk("Connected\n");
    }
  }

  void ConnectionManager::disconnected(struct bt_conn* conn, uint8_t reason)
  {
    s_conn = conn;
    printk("Disconnected (reason 0x%02x)\n", reason);
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

}