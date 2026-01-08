#pragma once
#include <Arduino.h>
#include <Firebase_ESP_Client.h>

#define GETBIT(variable,bit_number)   (variable<<bit_number) &1
#define WRITEBIT(variable,bit_number,value)  variable =  ((variable & ~(1<<bit_number))|(value)<<(bit_number))
#define TOGGLEBIT(variable,bit_number)    variable ^= (1<<(bit_number))
// desired/actual caches
extern bool desiredInited;
extern uint8_t lastDesired;
extern uint8_t lastActual;

// feedback filtering state (mirrors your original globals)
extern uint8_t fbCandidate;
extern uint8_t fbStable;
extern uint8_t fbCount;

// timeout array (as in your original code)
extern uint8_t DesiredTimeout[9];

// ignore flags
extern uint8_t     ignoreDesiredEvent;
extern uint32_t ignoreUntilMs[9];

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
