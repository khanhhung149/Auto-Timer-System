#pragma once
#include "datamodel.h"

namespace RelayController {
    void begin(const AppConfig& cfg);
    void set(uint8_t ch, bool on);
    bool isOn(uint8_t ch);
    void setActiveLow(uint8_t ch, bool activeLow);
}