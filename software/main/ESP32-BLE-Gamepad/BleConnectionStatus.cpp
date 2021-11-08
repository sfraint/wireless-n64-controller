#include "BleConnectionStatus.h"

BleConnectionStatus::BleConnectionStatus(void) {
}

void BleConnectionStatus::onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc)
{
  // Set very low latency connection parameters for more reponsive gamepad
  pServer->updateConnParams(desc->conn_handle, 6, 8, 0, 400);
  this->connected = true;
}

void BleConnectionStatus::onDisconnect(NimBLEServer* pServer)
{
  this->connected = false;
}
