#pragma once
#include <Arduino.h>
#include <RTClib.h>

namespace TimeManager{
    bool begin();
    bool isRtcOk();
    void setOffsets(int32_t gmtOffset, int32_t dstOffet);
    bool syncfromNTP(const char* server);
    DateTime now();
    void setMaunal(const DateTime& dt);
}