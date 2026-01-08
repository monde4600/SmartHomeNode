#pragma once
#include <Arduino.h>

void feedback_beginPins();
uint8_t readFeedback165_raw();
uint8_t readFeedbackStable(uint8_t stableCount = 3, uint16_t stepMs = 5);
