#pragma once
#include <Arduino.h>
#include <Firebase_ESP_Client.h>
// desired/actual caches
extern bool desiredInited;
extern uint8_t lastDesired;
extern uint8_t lastActual;

// feedback filtering state (mirrors your original globals)
extern uint8_t fbCandidate;
extern uint8_t fbStable;
extern uint8_t fbCount;

// timeout array (as in your original code)
extern uint8_t DesiredTimeout[8];

// ignore flags
extern uint8_t     ignoreDesiredEvent;
extern uint32_t ignoreUntilMs[8];

// stream error counter
extern uint64_t StreamFailError;

// Initialize desired cache
void initDesiredCacheOnce();

// Boot sync from feedback (writes actual + desired sync)
void bootSyncFromFeedback();

// Stream setup & polling
bool beginStreamOnPath(const String& streamPath);
void StreamCallFun();

// Token refresh / stream recover
void RefeishFirebase();

// Timeouts sync helper
void synkdesiredtoactualaftertimeout();

// Write helpers (used by main loop)
void writeActualBit(uint8_t ch, bool on);
void syncDesiredToActual(uint8_t ch, bool on);
