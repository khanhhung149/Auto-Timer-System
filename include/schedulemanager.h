#pragma once
#include "datamodel.h"
#include "RTClib.h"

namespace ScheduleManager {
    void update(const AppConfig& cfg, const DateTime& now);
    bool shouldBeOn(const RelayConfig& rc, const DateTime& now);

    // Su kien lich gan nhat sap toi (de hien tren OLED)
    struct NextEvent {
        bool     valid    = false;
        uint8_t  relay    = 0;      // kenh
        bool     turnOn   = false;  // true=sap BAT, false=sap TAT
        uint8_t  hour     = 0;
        uint8_t  min      = 0;
        uint16_t inMinutes = 0;     // con bao nhieu phut nua
    };
    NextEvent nextEvent(const AppConfig& cfg, const DateTime& now);
}