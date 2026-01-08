#pragma once
#include <Arduino.h>

// Relay output byte (bit0=ch1 .. bit7=ch8)
extern volatile uint8_t relayOut;

// channel mask helper

void relayIO_beginPins();
void writeRelays595(uint8_t value);
void toggleRelay(uint8_t ch);
