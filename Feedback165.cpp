#include "Feedback165.h"
#include "Pins.h"
#include "AppConfig.h"

void feedback_beginPins()
{
  pinMode(PIN_165_DATA, INPUT);
  pinMode(PIN_165_CLK, OUTPUT);
  pinMode(PIN_165_LOAD, OUTPUT);

  digitalWrite(PIN_165_LOAD, HIGH);
}

uint8_t readFeedback165_raw()
{
  digitalWrite(PIN_165_LOAD, LOW);
  delayMicroseconds(1);
  digitalWrite(PIN_165_LOAD, HIGH);
  delayMicroseconds(1);
  uint8_t v = shiftIn(PIN_165_DATA, PIN_165_CLK, MSBFIRST);

  #if (FEEDBACK_ACTIVE_LOW)
  v = ~v;
  #endif
  return v;
}

// قراءة ثابتة للبوت (3 قراءات ثابتة)
uint8_t readFeedbackStable(uint8_t stableCount, uint16_t stepMs)
{
  uint8_t candidate = readFeedback165_raw();
  uint8_t count = 1;

  while (count < stableCount) {
    delay(stepMs);
    uint8_t v = readFeedback165_raw();
    if (v == candidate) {
      count++;
    } else {
      candidate = v;
      count = 1;
    }
  }
  return candidate;
}
