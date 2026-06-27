#pragma once
#include "datamodel.h"

namespace WebServerManager {
  void begin(AppConfig& cfg);   // dang ky route + phuc vu /data tu LittleFS
  void loop();                  // goi trong loop() de xu ly client
}
