#include "timemanager.h"
#include "config.h"
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "time.h"     // configTime / getLocalTime / struct tm (ESP32 core)

static RTC_DS1307 rtc;
static bool       rtcOk = false;
static int32_t    gmtOffsetSec = DEFAULT_GMT_OFFSET;
static int32_t    dstOffsetSec = DEFAULT_DST_OFFSET;

bool TimeManager::begin() {
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  rtcOk = rtc.begin();
  if (!rtcOk) {
    Serial.println("[Time] Khong tim thay DS1307");
    return false;
  }
  if (!rtc.isrunning()) {
    Serial.println("[Time] DS1307 chua chay -> dat gio bien dich");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  Serial.println("[Time] DS1307 san sang");
  return true;
}

bool TimeManager::isRtcOk() { return rtcOk; }

void TimeManager::setOffsets(int32_t gmtOffset, int32_t dstOffset) {
  gmtOffsetSec = gmtOffset;
  dstOffsetSec = dstOffset;
}

// ---- Phuong an 1: NTP qua configTime (UDP 123) ----
static bool syncNTP(const char* server, int32_t offset) {
  configTime(offset, 0, server, "pool.ntp.org", "time.google.com");
  struct tm ti;
  for (int i = 0; i < 2; i++) {            // cho toi ~2s (du de mang cho phep NTP)
    if (getLocalTime(&ti, 1000)) {
      DateTime dt(ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday,
                  ti.tm_hour, ti.tm_min, ti.tm_sec);
      if (rtcOk) rtc.adjust(dt);
      Serial.printf("[Time] NTP OK: %04d-%02d-%02d %02d:%02d:%02d\n",
                    dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());
      return true;
    }
    Serial.printf("[Time] Cho NTP %d/2...\n", i + 1);
  }
  return false;
}

// Phan tich header HTTP Date: "Wed, 24 Jun 2026 03:22:43 GMT" -> DateTime (UTC)
static bool parseHttpDate(const String& s, DateTime& outUtc) {
  char mon[4] = {0};
  int d, y, hh, mm, ss;
  if (sscanf(s.c_str(), "%*3s, %d %3s %d %d:%d:%d", &d, mon, &y, &hh, &mm, &ss) != 6) return false;
  const char* months = "JanFebMarAprMayJunJulAugSepOctNovDec";
  const char* p = strstr(months, mon);
  if (!p) return false;
  int month = (int)((p - months) / 3) + 1;
  outUtc = DateTime((uint16_t)y, (uint8_t)month, (uint8_t)d, (uint8_t)hh, (uint8_t)mm, (uint8_t)ss);
  return true;
}

// ---- Phuong an 2: lay gio tu header HTTP Date (cong 80, it bi chan) ----
static bool syncHTTP(int32_t offset) {
  HTTPClient http;
  WiFiClient client;
  http.setConnectTimeout(4000);
  http.begin(client, "http://www.google.com/generate_204");
  const char* hdrs[] = { "Date" };
  http.collectHeaders(hdrs, 1);
  http.GET();
  String dateStr = http.header("Date");
  http.end();
  if (dateStr.length() == 0) {
    Serial.println("[Time] HTTP khong co header Date");
    return false;
  }
  DateTime utc;
  if (!parseHttpDate(dateStr, utc)) {
    Serial.printf("[Time] Khong phan tich duoc Date: %s\n", dateStr.c_str());
    return false;
  }
  DateTime local = utc + TimeSpan(offset);   // UTC -> gio dia phuong
  if (rtcOk) rtc.adjust(local);
  Serial.printf("[Time] HTTP-time OK: %04d-%02d-%02d %02d:%02d:%02d\n",
                local.year(), local.month(), local.day(),
                local.hour(), local.minute(), local.second());
  return true;
}

bool TimeManager::syncfromNTP(const char* server) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[Time] Chua co wifi -> bo qua dong bo");
    return false;
  }
  int32_t offset = gmtOffsetSec + dstOffsetSec;

  if (syncNTP(server, offset)) return true;   // thu NTP truoc

  Serial.println("[Time] NTP that bai -> thu lay gio qua HTTP...");
  if (syncHTTP(offset)) return true;          // fallback HTTP

  Serial.println("[Time] Dong bo that bai (giu gio DS1307 hien co)");
  return false;
}

DateTime TimeManager::now() {
  if (rtcOk) return rtc.now();
  return DateTime((uint32_t)0);
}

void TimeManager::setMaunal(const DateTime& dt) {
  if (rtcOk) rtc.adjust(dt);
}
