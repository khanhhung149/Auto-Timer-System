#pragma once
#include "datamodel.h"

namespace ButtonManager {
  void begin(AppConfig& cfg);   // cau hinh 2 chan nut
  void loop();                  // doc nut, xu ly giu-lau cho nut reset
  bool consumeCyclePress();     // true 1 lan khi co nhan ngan nut cuon
  bool isResetHolding();        // true khi dang giu nut reset (main khong ve OLED)
}
