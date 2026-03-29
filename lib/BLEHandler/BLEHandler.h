#pragma once

#include <M5Core2.h>

// ====================== PUBLIC API (use these in main.cpp) ======================
void BLE_Begin(bool asClient); // call once after pickIsClient()
void BLE_Send(String message); // both roles — just call this
bool BLE_HasNewData();         // check if something arrived
String BLE_Read();             // returns the last received string (clears flag)
bool BLE_IsConnected();        // useful for status
void doHeartbeat(boolean isClient);