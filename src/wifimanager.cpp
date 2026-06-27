#include "wifimanager.h"
#include "config.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <DNSServer.h>

static bool apMode = false;
static DNSServer dnsServer;   // captive portal: tra moi ten mien ve IP cua AP

void WiFiManager::begin(const AppConfig& cfg) {
  // Thu che do STA neu da co SSID
  if (strlen(cfg.wifi_ssid) > 0) {
    Serial.printf("[WiFi] Dang ket noi '%s' ...\n", cfg.wifi_ssid);
    WiFi.mode(WIFI_STA);

    // IP tinh (neu tat DHCP). Phai goi WiFi.config TRUOC WiFi.begin
    if (!cfg.dhcp) {
      IPAddress ip, gw, sn, dns;
      if (ip.fromString(cfg.ip) && gw.fromString(cfg.gateway) && sn.fromString(cfg.subnet)) {
        dns.fromString(cfg.dns);
        WiFi.config(ip, gw, sn, dns);
        Serial.printf("[WiFi] IP tinh: %s\n", cfg.ip);
      } else {
        Serial.println("[WiFi] IP tinh khong hop le -> dung DHCP");
      }
    }

    WiFi.begin(cfg.wifi_ssid, cfg.wifi_pass);

    unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED &&
           millis() - t0 < WIFI_CONNECT_TIMEOUT_MS) {
      delay(300);
      Serial.print(".");
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    apMode = false;
    Serial.printf("[WiFi] OK! IP = %s\n", WiFi.localIP().toString().c_str());
  } else {
    // Khong noi duoc -> phat AP de cau hinh
    apMode = true;
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    Serial.printf("[WiFi] Phat AP '%s' | mat khau '%s' | IP = %s\n",
                  AP_SSID, AP_PASSWORD, WiFi.softAPIP().toString().c_str());

    // Captive portal: moi truy van DNS -> tra ve IP cua AP (192.168.4.1)
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", WiFi.softAPIP());
    Serial.println("[WiFi] Captive portal DNS bat dau");
  }

  // mDNS: truy cap bang ten cho de
  const char* host = (strlen(cfg.mdns) > 0) ? cfg.mdns : MDNS_HOST;
  if (MDNS.begin(host)) {
    MDNS.addService("http", "tcp", WEB_PORT);
    Serial.printf("[WiFi] Truy cap: http://%s.local\n", host);
  }
}

void WiFiManager::loop() {
  if (apMode) dnsServer.processNextRequest();
}

bool WiFiManager::isConnected() { return WiFi.status() == WL_CONNECTED; }
bool WiFiManager::isAP()        { return apMode; }
String WiFiManager::ip() {
  return apMode ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
}
