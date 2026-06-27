#pragma once
#include <Arduino.h>

//so kenh relay
#define RELAY_COUNT 4
#define SCHEDULES_PER_RELAY 3

//chan dieu khien 4 relay (tranh 26/27 da danh cho OLED DC/RST)
static const uint8_t RELAY_PINS[RELAY_COUNT] = {25, 32, 33, 13};
#define RELAY_ACTIVE_LOW_DEFAULT true

//DS1307-I2C
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

//OLED SSD1309 - SPI
#define OLED_SCK_PIN 18
#define OLED_MOSI_PIN 23
#define OLED_CS_PIN 5
#define OLED_DC_PIN 26
#define OLED_RST_PIN 27
 
//WIFI
#define AP_SSID "AutoTimer-Setup"
#define AP_PASSWORD ""
#define MDNS_HOST "autotimer"
#define WIFI_CONNECT_TIMEOUT_MS 15000

// WiFi nha de TEST (tam thoi) - de "" neu khong dung
#define DEFAULT_WIFI_SSID "EETIUM WORKSPACE"
#define DEFAULT_WIFI_PASS "@1223334444"

//NTP
#define DEFAULT_NTP_SERVER "pool.ntp.org"
#define DEFAULT_GMT_OFFSET 25200
#define DEFAULT_DST_OFFSET 0

//cau hinh littleFS
#define CONFIG_PATH "/config.json"

//chu ky lap(millis)
#define SCHEDULE_EVAL_INTERVAL_MS 1000
#define DISPLAY_REFRESH_INTERVAL_MS 500
#define WEB_PORT 80

//Nut bam vat ly (INPUT_PULLUP, nut noi xuong GND)
#define BTN_CYCLE_PIN 4      // cuon man OLED: dong ho -> QR -> relay1..4 -> lai
#define BTN_RESET_PIN 19     // giu 5s = clear WiFi; giu 30s = reset all
#define BTN_DEBOUNCE_MS 40
#define RESET_CLEAR_MS 5000      // giu 5s -> xoa WiFi
#define RESET_FACTORY_MS 30000   // giu 30s -> reset toan bo
#define DISPLAY_AUTO_RETURN_MS 20000  // khong bam 20s -> tu ve dong ho
