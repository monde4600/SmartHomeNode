#include"bit_math.h"
#include "RelayIO.h"
#include "Pins.h"
#include "AppConfig.h"
#include "RelayStorage.h"

volatile uint8_t relayOut = 0x00;

void relayIO_beginPins()
{
  pinMode(PIN_595_DATA, OUTPUT);
  pinMode(PIN_595_CLK, OUTPUT);
  pinMode(PIN_595_LATCH, OUTPUT);

  digitalWrite(PIN_595_LATCH, HIGH);
}

void writeRelays595(uint8_t value)
{
  uint8_t toSend = value;
  #if (RELAY_ACTIVE_LOW)
  toSend = ~toSend;
  #endif

  digitalWrite(PIN_595_LATCH, LOW);
  shiftOut(PIN_595_DATA, PIN_595_CLK, MSBFIRST, toSend);
  digitalWrite(PIN_595_LATCH, HIGH);
}

void toggleRelay(uint8_t ch)
{
  TOGGLEBIT(relayOut,ch);
  writeRelays595(relayOut);

  // ✅ حفظ آخر حالة للريلايات (مؤجل)
  requestSaveRelayOut();
}
