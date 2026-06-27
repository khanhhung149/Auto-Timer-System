#pragma once
#include "datamodel.h"
#include <Arduino.h>

namespace WiFiManager{
    void begin(const AppConfig& cfg);
    void loop();                 // xu ly DNS captive portal khi o che do AP
    bool isConnected();
    bool isAP();
    String ip();
}