#include"bit_math.h"
#include <Arduino.h>
#include <WiFi.h>

#include <Firebase_ESP_Client.h>

#include "addons/RTDBHelper.h"

#include "AppConfig.h"
#include "Pins.h"
#include "Paths.h"
#include "RelayIO.h"
#include "RelayStorage.h"
#include "Feedback165.h"
#include "FirebaseService.h"
#include "StreamService.h"

// ===================== MAIN =====================
String streamPath = basePath();

uint16_t Fiveminute = 30;

void setup() {
  Serial.begin(115200);

  // Pins init
  relayIO_beginPins();
  feedback_beginPins();


  // ✅ استرجاع آخر حالة للريلايات بدل تصفيرها
  uint8_t stored = 0x00;
  if (loadRelayOut(stored)) {
    relayOut = stored;
  } else {
    relayOut = 0x00;
  }
  writeRelays595(relayOut);
  lastSavedRelayOut = relayOut; // عشان مايحاولش يحفظها تاني فوراً

  // WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");

  // Firebase
  firebase_begin();

  // اقرأ desired الحالي (علشان ما نعملش toggle عند البوت)
  initDesiredCacheOnce();

  // ✅ قبل ما نبدأ Stream: اعمل Boot Sync
  bootSyncFromFeedback();

  // Start stream on channels root
  beginStreamOnPath(streamPath);

}

void loop() {

  // فك ignore flags بعد انتهاء الوقت
  for (uint8_t ch = 0; ch <= 7; ch++) {
    if (GETBIT(ignoreDesiredEvent,ch) && (int32_t)(ignoreUntilMs[ch] - millis()) <= 0) {
      WRITEBIT(ignoreDesiredEvent,ch,false);
    }
  }

  // كل 20ms: اقرأ raw وفلتره للاستقرار
  static uint32_t t = 0;
  static uint32_t t2 = 0;
  if (millis() - t >= FB_SAMPLE_PERIOD_MS)
  {
    t = millis();

    // stream
    StreamCallFun();

    fbCandidate = readFeedbackStable(FB_STABLE_COUNT, FB_SAMPLE_PERIOD_MS);

    // لما تثبت القراءة: لو اختلفت عن stable -> حدّث القنوات المتغيرة فقط
    if ( fbStable != fbCandidate) 
    {
      uint8_t changed = fbStable ^ fbCandidate;
      fbStable = fbCandidate;

      for (uint8_t ch = 0; ch <= 7; ch++) 
      {
        if (GETBIT(changed ,ch)) {
          bool on = GETBIT(fbStable,ch);

          // ✅ update only changed channels
          writeActualBit(ch, on);
          syncDesiredToActual(ch, on);
        }
      }
    }
  }

  if (millis() - t2 >= TimeOutSynk)
  {
    static uint8_t RefrishingRateInS = 100;
    t2 = millis();
    synkdesiredtoactualaftertimeout();

    if(--Fiveminute==0)
    {
      Serial.print(ESP.getFreeHeap());
      Serial.println("..");
      Firebase.RTDB.getBool(&fbdo, desiredPath(9).c_str());
      Fiveminute = 30;

      if(--RefrishingRateInS==0)
      {
        RefrishingRateInS = 100; //50 minuits
        RefeishFirebase();
      }
    }
  }

  if(StreamFailError>MAX_STREAMERRORS)
  {
    StreamFailError= 0;
    RefeishFirebase();
  }

  // ✅ تنفيذ حفظ الريلايات (مؤجل)
  handleRelaySave();
}
