#include "Paths.h"
#include "AppConfig.h"

String basePath() {
  return String("device_states/") + DEVICE_ID + "/channels";
}

String desiredPath(uint8_t ch) {
  return basePath() + "/" + String(ch) + "/desired";
}

String actualPath(uint8_t ch) {
  return basePath() + "/" + String(ch) + "/actual";
}
