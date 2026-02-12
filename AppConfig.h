#pragma once

// ===================== WIFI =====================
#define WIFI_SSID     "OPPO"
#define WIFI_PASS     "1234567890"

// ===================== FIREBASE =====================
#define API_KEY       "AIzaSyDQt4wIFuKMwDAA16EmvmRm-EfeSQeEQ8o"
#define DATABASE_URL  "https://smarthomeapp-04-12-2025-default-rtdb.europe-west1.firebasedatabase.app/"  // ✅ لازم تنتهي بـ /
#define USER_EMAIL    "abanob.helmy@smarthome.com"
#define USER_PASSWORD "123456"

#define DEVICE_ID     "device_01"

// ===================== SETTINGS =====================
static const bool RELAY_ACTIVE_LOW    = false; // true لو الريلاي Active LOW
static const bool FEEDBACK_ACTIVE_LOW = false; // true لو الفيدباك Active LOW

// فلتر الثبات للفيدباك (يمنع رعشة القنوات)
#define FB_SAMPLE_PERIOD_MS 5
#define FB_STABLE_COUNT     3

/*start Time out manually imp*/
#define TimerDeactivate      0xFF
#define DesiredTimeoutinS    3
#define MAX_STREAMERRORS     3
//function call every 1000ms
#define TimeOutSynk          1000
#define STREAMTICKTIME       100 //TICK one tick is 5ms
/*end Time out manually imp*/

// Relay save (ESP32 NVS Preferences)
#define RELAY_SAVE_DEBOUNCE_MS  500
