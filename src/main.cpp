#include <Arduino.h>
#include "config.h"
#include "datamodel.h"
#include "storage.h"
#include "timemanager.h"
#include "relaycontroller.h"
#include "schedulemanager.h"
#include "displaymanager.h"
#include "wifimanager.h"
#include "webservermanager.h"
#include "buttonmanager.h"
#include "weathermanager.h"

// Cau hinh toan cuc, song suot chuong trinh
AppConfig config;

// Moc thoi gian cho cac tac vu dinh ky
static unsigned long lastSched = 0;
static unsigned long lastDisp  = 0;
static unsigned long lastNtp   = 0;

// Trang thai man OLED: 0=dong ho, 1=QR, 2..5=cau hinh relay 1..4
static uint8_t       displayMode  = 0;
static unsigned long lastInteract = 0;

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== Auto Timer System ===");

  // 1) Man hinh truoc de bao trang thai khoi dong
  DisplayManager::begin();
  DisplayManager::showBoot("Đang khởi động...");

  // 2) Nap cau hinh tu NVS
  Storage::begin();
  Storage::load(config);

  // Ap ngon ngu cho OLED theo cau hinh
  DisplayManager::setLang(config.lang);

  // 3) Dong ho DS1307
  TimeManager::begin();
  TimeManager::setOffsets(config.gmt_offset, config.dst_offset);

  // 4) Relay (tat het luc dau)
  RelayController::begin(config);

  // 5) Mang: STA hoac AP
  WiFiManager::begin(config);
  DisplayManager::showBoot(WiFiManager::ip().c_str());

  // 6) Co mang thi dong bo gio ngay
  if (WiFiManager::isConnected()) {
    TimeManager::syncfromNTP(config.ntpServer);
  }

  // 7) May chu web phuc vu giao dien + API
  WebServerManager::begin(config);

  // 8) Nut bam vat ly (cuon man + reset)
  ButtonManager::begin(config);

  // 9) Thoi tiet (do thanh pho + nhiet do)
  WeatherManager::begin(config);

  Serial.println("=== San sang! ===");
}

void loop() {
  WiFiManager::loop();                  // DNS captive portal (khi o AP)
  WebServerManager::loop();             // xu ly request web
  ButtonManager::loop();                // doc 2 nut bam
  WeatherManager::loop();               // cap nhat thoi tiet dinh ky
  unsigned long now = millis();

  // Nut cuon: chuyen man (dong ho -> QR -> relay1..4 -> lai)
  if (ButtonManager::consumeCyclePress()) {
    displayMode = (displayMode + 1) % (2 + RELAY_COUNT);   // 0..5
    lastInteract = now;
  }
  // Khong bam lau -> tu ve dong ho
  if (displayMode != 0 && now - lastInteract > DISPLAY_AUTO_RETURN_MS) displayMode = 0;

  // Danh gia lich moi 1s -> bat/tat relay
  if (now - lastSched >= SCHEDULE_EVAL_INTERVAL_MS) {
    lastSched = now;
    ScheduleManager::update(config, TimeManager::now());
  }

  // Cap nhat OLED moi 0.5s (bo qua khi dang giu nut reset - no tu ve thong bao)
  if (!ButtonManager::isResetHolding() && now - lastDisp >= DISPLAY_REFRESH_INTERVAL_MS) {
    lastDisp = now;
    if (WiFiManager::isAP()) {
      // Che do AP setup -> hien huong dan (kieu he cu)
      DisplayManager::showApSetup(AP_SSID, WiFiManager::ip().c_str());
    } else {
      DateTime t = TimeManager::now();
      if (displayMode == 0) {
        ScheduleManager::NextEvent ev = ScheduleManager::nextEvent(config, t);
        String w = WeatherManager::hasData() ? WeatherManager::footer() : "";
        DisplayManager::showMain(t, WiFiManager::isConnected(), w.c_str(),
                                 ev.valid, ev.relay, ev.turnOn, ev.hour, ev.min);
      } else if (displayMode == 1) {
        String ip = WiFiManager::ip();
        DisplayManager::showQR("http://" + ip, ip);
      } else {
        uint8_t idx = displayMode - 2;    // 0..3
        DisplayManager::showRelaySchedule(idx, config.relays[idx]);
      }
    }
  }

  // Dong bo lai NTP moi 1 gio (neu con mang)
  if (WiFiManager::isConnected() && now - lastNtp >= 3600000UL) {
    lastNtp = now;
    TimeManager::syncfromNTP(config.ntpServer);
  }
}
