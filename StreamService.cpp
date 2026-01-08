#include "StreamService.h"
#include "FirebaseService.h"
#include "Paths.h"
#include "RelayIO.h"
#include "Feedback165.h"
#include "AppConfig.h"

#define BITMASK(variable,bit_number)   (variable<<bit_number) &1

// ===================== STATE (same as your original globals) =====================
uint8_t fbCandidate = 0;
uint8_t fbStable    = 0;
uint8_t fbCount     = 0;

bool desiredInited = false;
bool lastDesired[9]; // 1..8
bool lastActual[9];  // 1..8

uint8_t DesiredTimeout[9] = {TimerDeactivate}; // (kept exactly as you had)

// منع loop لما النود يكتب desired
bool     ignoreDesiredEvent[9];
uint32_t ignoreUntilMs[9];

// ===================== STREAM =====================
uint64_t StreamFailError = 0;
static uint8_t streamTime = STREAMTICKTIME;

// ===================== FIREBASE HELPERS =====================

// ✅ يكتب actual فقط لو اتغير فعلاً
void writeActualBit(uint8_t ch, bool on)
{
  if (lastActual[ch] == on) return;
  lastActual[ch] = on;
  if(!Firebase.RTDB.setBool(&fbdo, actualPath(ch).c_str(), on))
  {
    lastActual[ch] ^= 1;
    fbStable ^= 1<<(ch-1);
  }
}

// ✅ يخلي desired = actual فقط لو مختلف + يمنع loop
void syncDesiredToActual(uint8_t ch, bool on)
{
  if (lastDesired[ch] == on) return;

  ignoreDesiredEvent[ch] = true;
  ignoreUntilMs[ch] = millis() + 400; // تجاهل event لمدة 0.4 ثانية

  lastDesired[ch] = on;
  if(!Firebase.RTDB.setBool(&fbdo, desiredPath(ch).c_str(), on))
  {
    lastDesired[ch] ^= 1;
    DesiredTimeout[ch]=DesiredTimeoutinS;
  }
}

static bool parseChannelFromStreamPath(const String &path, uint8_t &ch, bool &isDesired)
{
  // "/1/desired"
  if (!path.startsWith("/")) return false;
  int secondSlash = path.indexOf('/', 1);
  if (secondSlash < 0) return false;

  int chInt = path.substring(1, secondSlash).toInt();
  if (chInt < 1 || chInt > 8) return false;

  String key = path.substring(secondSlash + 1);
  isDesired = (key == "desired");
  ch = (uint8_t)chInt;
  return true;
}

static void onStreamCallback(FirebaseStream data)
{
  String path = data.dataPath();
  String type = data.dataType();

  uint8_t ch = 0;
  bool isDesired = false;
  if (!parseChannelFromStreamPath(path, ch, isDesired)) return;
  if (!isDesired) return;

  // ✅ تجاهل أي desired event النود هو اللي كاتبه (sync)
  if (ignoreDesiredEvent[ch] && (int32_t)(ignoreUntilMs[ch] - millis()) > 0) {
    // حدّث الكاش فقط
    if (type == "boolean") lastDesired[ch] = data.boolData();
    else if (type == "int") lastDesired[ch] = (data.intData() != 0);
    else if (type == "float") lastDesired[ch] = (data.floatData() != 0);
    else if (type == "string") {
      String s = data.stringData();
      lastDesired[ch] = (s == "1" || s == "true" || s == "TRUE");
    }
    return;
  }

  bool desiredVal = false;
  if (type == "boolean") desiredVal = data.boolData();
  else if (type == "int") desiredVal = (data.intData() != 0);
  else if (type == "float") desiredVal = (data.floatData() != 0);
  else if (type == "string") {
    String s = data.stringData();
    desiredVal = (s == "1" || s == "true" || s == "TRUE");
  } else return;

  // أول مرة: cache فقط (من غير Toggle)
  if (!desiredInited) {
    lastDesired[ch] = desiredVal;
    return;
  }

  // ✅ أي تغيير في desired = أمر Toggle
  if (desiredVal != lastDesired[ch]) {
    lastDesired[ch] = desiredVal;
    toggleRelay(ch);
    DesiredTimeout[ch]=DesiredTimeoutinS;
    // actual + desired-sync هيتموا لما feedback يثبت
  }
}

static void onStreamTimeout(bool timeout)
{
  if (timeout) Serial.println("Stream timeout, resuming...");
  if (!stream.httpConnected()) {
    Serial.printf("Stream error: %s\n", stream.errorReason().c_str());
  }
}

void initDesiredCacheOnce()
{
  for (uint8_t ch = 1; ch <= 8; ch++) {
    ignoreDesiredEvent[ch] = false;
    ignoreUntilMs[ch] = 0;

    lastActual[ch] = false; // baseline

    if (Firebase.RTDB.getBool(&fbdo, desiredPath(ch).c_str())) {
      lastDesired[ch] = fbdo.boolData();
    } else {
      lastDesired[ch] = false;
    }
  }
  desiredInited = true;
  Serial.println("Desired cache initialized (no toggle on boot).");
}

// ===================== BOOT SYNC =====================
// أول ما النود يوصل: يبعت actual لكل القنوات + يساوي desired = actual
void bootSyncFromFeedback()
{
  uint8_t fb = readFeedbackStable(FB_STABLE_COUNT, FB_SAMPLE_PERIOD_MS);

  // init filter state
  fbCandidate = fb;
  fbStable = fb;
  fbCount = FB_STABLE_COUNT;

  // اكتب actual + sync desired (مرة واحدة)
  for (uint8_t ch = 1; ch <= 8; ch++) {
    bool on = (fb & chMask(ch)) != 0;

    // اجبار كتابة actual مرة واحدة
    lastActual[ch] = !on;
    writeActualBit(ch, on);

    // sync desired = actual (لو مختلف)
    if (lastDesired[ch] != on) {
      syncDesiredToActual(ch, on);
    }
  }

  Serial.print("Boot feedback stable = ");
  Serial.println(fb, BIN);
  Serial.println("Boot sync done (actual written + desired synced).");
}

bool beginStreamOnPath(const String& streamPath)
{
  
  if (!Firebase.RTDB.beginStream(&stream, streamPath.c_str())) {
    Serial.printf("Stream begin error: %s\n", stream.errorReason().c_str());
    return false;
  }

  Firebase.RTDB.setMaxRetry(&stream, 3);
  Firebase.RTDB.setStreamCallback(&stream, onStreamCallback, onStreamTimeout,4096);
  Serial.println("Stream started: " + streamPath);
  return true;
}

// original polling function (called in loop)
void StreamCallFun()
{
  if(--streamTime==0)
  {
    streamTime = STREAMTICKTIME;
    if(!Firebase.RTDB.readStream(&stream))
    {
      StreamFailError++;
      Serial.print("Stream failure Error");
      Serial.print(StreamFailError);
      Serial.println(" times");
      Serial.printf("Stream begin error, %s\n", stream.errorReason().c_str());
    }
    else if(StreamFailError!=0)
    {
      Firebase.RTDB.clearErrorQueue(&stream);
      StreamFailError--;
    }
  }
}

//function call every 1000ms
void synkdesiredtoactualaftertimeout()
{
  for(uint8_t ch=1;ch<=8;ch++)
  {
    if(DesiredTimeout[ch]!=TimerDeactivate)
    {
      if(--DesiredTimeout[ch]==0)
      {
        DesiredTimeout[ch]=TimerDeactivate;
        lastDesired[ch]=lastActual[ch];
        if(!Firebase.RTDB.setBool(&fbdo, desiredPath(ch).c_str(), lastActual[ch]))
        {
          lastDesired[ch] ^= 1;
          DesiredTimeout[ch]=DesiredTimeoutinS;
        }
      }
    }
  }
}

static void StreamRecover()
{
  // re-set stream callbacks (same path)
  beginStreamOnPath(basePath());
}

uint8_t RefrishingRateInS = 100; //50 minuits
void RefeishFirebase()
{
  Serial.println("Forcing token refresh...");
  Firebase.refreshToken(&config);

  while (!Firebase.ready())
  {
    Serial.println("SNR");
    delay(10);
  }

  if (Firebase.ready())
  {
    StreamRecover();
  } else {
    Serial.println("Token refresh failed. Check network/credentials.");
  }
}
