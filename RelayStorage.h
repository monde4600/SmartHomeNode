#pragma once
#include <Arduino.h>

// Load/save relayOut state (currently stored in ESP32 NVS using Preferences)
bool loadRelayOut(uint8_t &out);
bool saveRelayOut(uint8_t value);

// Debounced save helpers
void requestSaveRelayOut();
void handleRelaySave();

extern uint8_t lastSavedRelayOut;
