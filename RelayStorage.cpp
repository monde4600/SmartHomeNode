#include "RelayStorage.h"
#include "AppConfig.h"
#include "RelayIO.h"
#include <Preferences.h>

static Preferences prefs;

uint8_t lastSavedRelayOut = 0x00;

static bool     relaySavePending = false;
static uint32_t relaySaveAtMs = 0;

bool loadRelayOut(uint8_t &out)
{
  if (!prefs.begin("relay", true)) return false;   // read-only
  out = prefs.getUChar("state", 0x00);
  prefs.end();
  return true;
}

bool saveRelayOut(uint8_t value)
{
  if (!prefs.begin("relay", false)) return false;  // read-write
  prefs.putUChar("state", value);
  prefs.end();
  return true;
}

void requestSaveRelayOut()
{
  if (relayOut == lastSavedRelayOut) return;
  relaySavePending = true;
  relaySaveAtMs = millis() + RELAY_SAVE_DEBOUNCE_MS;
}

void handleRelaySave()
{
  if (!relaySavePending) return;
  if ((int32_t)(relaySaveAtMs - millis()) > 0) return;

  if (saveRelayOut(relayOut)) {
    lastSavedRelayOut = relayOut;
  }
  relaySavePending = false;
}
