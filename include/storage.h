#pragma once
#include "datamodel.h"
#include <ArduinoJson.h>

namespace Storage {
  bool begin();
  void setDefaults(AppConfig& cfg);
  bool load(AppConfig& cfg);            // doc config tu NVS -> cfg
  bool save(const AppConfig& cfg);      // ghi cfg -> NVS
  void clearAll();                      // xoa sach config trong NVS (factory reset)

  // Dung chung cho ca NVS lan API web:
  // includeSecrets=false -> bo mat khau (wifi_pass, web_pass) khi gui ra web
  void toJson(const AppConfig& cfg, JsonDocument& doc, bool includeSecrets = true);
  void fromJson(AppConfig& cfg, JsonDocument& doc);      // JSON -> cfg (giu gia tri cu neu thieu)
}
