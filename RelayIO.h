#pragma once
#include <Arduino.h>

// Relay output byte (bit0=ch1 .. bit7=ch8)
extern volatile uint8_t relayOut;

// channel mask helper
inline uint8_t chMask(uint8_t ch) { // 1..8
  return (ch >= 1 && ch <= 8) ? (1u << (ch - 1)) : 0;
}

void relayIO_beginPins();
void writeRelays595(uint8_t value);
void toggleRelay(uint8_t ch);
