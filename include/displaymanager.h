#pragma once
#include <RTClib.h>
#include "datamodel.h"

namespace DisplayManager {
    bool begin();
    void setLang(const char* lang);   // "vi" hoac "en" - doi nhan thu
    bool isEN();                      // dang dung tieng Anh?
    void showBoot(const char* msg);
    void showClock(const DateTime& now);
    void showRelays(const bool* states, const char* const* names, uint8_t count);

    // Man hinh chinh (kieu he cu): ngay goc trai, icon WiFi goc phai,
    // gio lon giua, footer = thoi tiet (neu co) hoac su kien ke tiep
    void showMain(const DateTime& now, bool wifiOn, const char* weather,
                  bool hasNext, uint8_t evRelay, bool evOn, uint8_t evHour, uint8_t evMin);

    // Man phu (nut cuon)
    void showMessage(const char* l1, const char* l2);          // 2 dong thong bao
    void showQR(const String& url, const String& ipText);      // QR + IP
    void showRelaySchedule(uint8_t idx, const RelayConfig& rc); // lich 1 relay
    void showApSetup(const char* ssid, const char* ip);        // man AP setup (kieu he cu)
}
