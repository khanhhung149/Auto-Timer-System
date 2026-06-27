#include "storage.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <Preferences.h>

// Cau hinh luu trong NVS (vung nho rieng, KHONG bi xoa khi uploadfs)
static Preferences prefs;
#define NVS_NS  "autotimer"
#define NVS_KEY "config"

// Mount LittleFS; tham so 'true' = neu loi thi tu format roi thu lai
bool Storage::begin() {
  if (!LittleFS.begin(true)) {
    Serial.println("[Storage] LittleFS mount THAT BAI");
    return false;
  }
  Serial.println("[Storage] LittleFS san sang");
  return true;
}

// Dat toan bo ve mac dinh + danh ten relay 1..4
void Storage::setDefaults(AppConfig& cfg) {
  cfg = AppConfig();   // reset theo gia tri mac dinh khai bao trong struct
  for (uint8_t i = 0; i < RELAY_COUNT; i++) {
    snprintf(cfg.relays[i].name, sizeof(cfg.relays[i].name), "Relay %u", i + 1);
  }
}

// ----- cfg -> JSON -----
void Storage::toJson(const AppConfig& cfg, JsonDocument& doc, bool includeSecrets) {
  doc["wifi_ssid"]  = cfg.wifi_ssid;
  if (includeSecrets) doc["wifi_pass"] = cfg.wifi_pass;
  doc["ntpServer"]  = cfg.ntpServer;
  doc["gmt_offset"] = cfg.gmt_offset;
  doc["dst_offset"] = cfg.dst_offset;
  doc["lang"]       = cfg.lang;
  doc["dhcp"]       = cfg.dhcp;
  doc["ip"]         = cfg.ip;
  doc["gateway"]    = cfg.gateway;
  doc["subnet"]     = cfg.subnet;
  doc["dns"]        = cfg.dns;
  doc["mdns"]       = cfg.mdns;
  doc["city"]       = cfg.city;
  doc["web_user"]   = cfg.web_user;
  if (includeSecrets) doc["web_pass"] = cfg.web_pass;

  JsonArray relays = doc["relays"].to<JsonArray>();
  for (uint8_t i = 0; i < RELAY_COUNT; i++) {
    const RelayConfig& rc = cfg.relays[i];
    JsonObject r = relays.add<JsonObject>();
    r["name"]      = rc.name;
    r["activeLow"] = rc.activeLow;
    r["mode"]      = (uint8_t)rc.mode;

    JsonArray scheds = r["schedules"].to<JsonArray>();
    for (uint8_t j = 0; j < SCHEDULES_PER_RELAY; j++) {
      const Schedule& sc = rc.schedules[j];
      JsonObject s = scheds.add<JsonObject>();
      s["name"]      = sc.name;
      s["enabled"]   = sc.enabled;
      s["startHour"] = sc.startHour;
      s["startMin"]  = sc.startMin;
      s["stopHour"]  = sc.stopHour;
      s["stopMin"]   = sc.stopMin;
      s["daysMask"]  = sc.daysMask;
    }
  }
}

// ----- JSON -> cfg (truong nao thieu thi giu nguyen gia tri cu) -----
void Storage::fromJson(AppConfig& cfg, JsonDocument& doc) {
  strlcpy(cfg.wifi_ssid, doc["wifi_ssid"] | cfg.wifi_ssid, sizeof(cfg.wifi_ssid));
  strlcpy(cfg.wifi_pass, doc["wifi_pass"] | cfg.wifi_pass, sizeof(cfg.wifi_pass));
  strlcpy(cfg.ntpServer, doc["ntpServer"] | cfg.ntpServer, sizeof(cfg.ntpServer));
  cfg.gmt_offset = doc["gmt_offset"] | cfg.gmt_offset;
  cfg.dst_offset = doc["dst_offset"] | cfg.dst_offset;
  strlcpy(cfg.lang, doc["lang"] | cfg.lang, sizeof(cfg.lang));
  cfg.dhcp = doc["dhcp"] | cfg.dhcp;
  strlcpy(cfg.ip,      doc["ip"]      | cfg.ip,      sizeof(cfg.ip));
  strlcpy(cfg.gateway, doc["gateway"] | cfg.gateway, sizeof(cfg.gateway));
  strlcpy(cfg.subnet,  doc["subnet"]  | cfg.subnet,  sizeof(cfg.subnet));
  strlcpy(cfg.dns,     doc["dns"]     | cfg.dns,     sizeof(cfg.dns));
  strlcpy(cfg.mdns,    doc["mdns"]    | cfg.mdns,    sizeof(cfg.mdns));
  strlcpy(cfg.city,    doc["city"]    | cfg.city,    sizeof(cfg.city));
  strlcpy(cfg.web_user, doc["web_user"] | cfg.web_user, sizeof(cfg.web_user));
  strlcpy(cfg.web_pass, doc["web_pass"] | cfg.web_pass, sizeof(cfg.web_pass));

  uint8_t ri = 0;
  for (JsonObject r : doc["relays"].as<JsonArray>()) {
    if (ri >= RELAY_COUNT) break;
    RelayConfig& rc = cfg.relays[ri];
    strlcpy(rc.name, r["name"] | rc.name, sizeof(rc.name));
    rc.activeLow = r["activeLow"] | rc.activeLow;
    rc.mode = (RelayMode)(uint8_t)(r["mode"] | (int)rc.mode);

    uint8_t si = 0;
    for (JsonObject s : r["schedules"].as<JsonArray>()) {
      if (si >= SCHEDULES_PER_RELAY) break;
      Schedule& sc = rc.schedules[si];
      strlcpy(sc.name, s["name"] | sc.name, sizeof(sc.name));
      sc.enabled   = s["enabled"]   | sc.enabled;
      sc.startHour = s["startHour"] | sc.startHour;
      sc.startMin  = s["startMin"]  | sc.startMin;
      sc.stopHour  = s["stopHour"]  | sc.stopHour;
      sc.stopMin   = s["stopMin"]   | sc.stopMin;
      sc.daysMask  = s["daysMask"]  | sc.daysMask;
      si++;
    }
    ri++;
  }
}

bool Storage::load(AppConfig& cfg) {
  setDefaults(cfg);   // luon bat dau tu mac dinh

  prefs.begin(NVS_NS, true);            // mo NVS che do chi doc
  String in = prefs.getString(NVS_KEY, "");
  prefs.end();

  if (in.length() == 0) {
    Serial.println("[Storage] Chua co config (NVS) -> dung mac dinh");
    return false;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, in);
  if (err) {
    Serial.printf("[Storage] JSON loi: %s -> mac dinh\n", err.c_str());
    return false;
  }
  fromJson(cfg, doc);
  Serial.println("[Storage] Da nap config (NVS)");
  return true;
}

bool Storage::save(const AppConfig& cfg) {
  JsonDocument doc;
  toJson(cfg, doc);

  String out;
  serializeJson(doc, out);

  prefs.begin(NVS_NS, false);           // mo NVS che do ghi
  prefs.putString(NVS_KEY, out);
  prefs.end();

  Serial.printf("[Storage] Da luu config (NVS, %u bytes)\n", out.length());
  return true;
}

void Storage::clearAll() {
  prefs.begin(NVS_NS, false);
  prefs.clear();
  prefs.end();
  Serial.println("[Storage] Da xoa sach config (NVS)");
}
