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
      log_msg("Bluetooth init failed (err {})", err);
      return false;
    }

    log_msg("Bluetooth init successfully");

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
      log_msg("Bluetooth init failed (err {})", err);
    }
  }

  bool ConnectionManager::start_adv()
  {
    int err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
      log_msg("Advertising failed to start (err {})", err);
      return false;
    }

    log_msg("Advertising successfully started");

    return true;
  }

  void ConnectionManager::connected(struct bt_conn* conn, uint8_t err)
  {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
    s_conn = conn;

    if (err) {
      log_msg("Connection failed (err {})\n", err);
      return;
    }

    log_msg("Connected");
    send_con_msg(ConnectionEvent::connected);

    if (bt_conn_set_security(conn, BT_SECURITY_L4)) {
      log_msg("Failed to set security");
    }
  }

  void ConnectionManager::disconnected(struct bt_conn* conn, uint8_t reason)
  {
    s_conn = conn;
    log_msg("Disconnected (reason {})\n", reason);
    send_con_msg(ConnectionEvent::disconnected);
  }

  void ConnectionManager::auth_passkey_display(struct bt_conn* conn, unsigned int passkey)
  {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    log_msg("Passkey for {}: {}", addr, passkey);
  }

  void ConnectionManager::auth_cancel(struct bt_conn* conn)
  {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    log_msg("Pairing cancelled: {}", addr);
  }

  void ConnectionManager::security_changed(struct bt_conn* conn, bt_security_t level, enum bt_security_err err)
  {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (!err) {
      log_msg("Security changed: {} level {}", addr, level);
    }
    else {
      log_msg("Security failed: {} level {} err {}", addr, level, err);
    }
  }

  void ConnectionManager::pairing_confirm(struct bt_conn* conn)
  {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    bt_conn_auth_pairing_confirm(conn);

    log_msg("Pairing confirmed: {}", addr);
  }

  void ConnectionManager::pairing_complete(struct bt_conn* conn, bool bonded)
  {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    log_msg("Pairing completed: {}, bonded: {}", addr, bonded);
  }

  void ConnectionManager::pairing_failed(struct bt_conn* conn, enum bt_security_err reason)
  {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    log_msg("Pairing failed conn: {}, reason {}", addr, reason);
  }

  void ConnectionManager::identity_resolved(struct bt_conn* conn, const bt_addr_le_t* rpa, const bt_addr_le_t* identity)
  {
    char addr_identity[BT_ADDR_LE_STR_LEN];
    char addr_rpa[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(identity, addr_identity, sizeof(addr_identity));
    bt_addr_le_to_str(rpa, addr_rpa, sizeof(addr_rpa));

    log_msg("Identity resolved {} -> {}", addr_rpa, addr_identity);
  }

  bt_conn* ConnectionManager::get_connection()
  {
    return s_conn;
  }


}

