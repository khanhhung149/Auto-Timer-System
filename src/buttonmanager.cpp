#include "buttonmanager.h"
#include "config.h"
#include "displaymanager.h"
#include "storage.h"
#include <Arduino.h>

static AppConfig* g_cfg = nullptr;
static bool cyclePending = false;

// Chon chuoi theo ngon ngu (giong TR ben display)
static const char* T(const char* vi, const char* en){ return DisplayManager::isEN() ? en : vi; }

// Debounce kieu "cho on dinh": trang thai chi doi sau khi tin hieu giu yen
// du BTN_DEBOUNCE_MS -> chong dội (bounce) khi cham day/nut nhieu.
static int cycRaw = HIGH, cycStable = HIGH;
static unsigned long cycChange = 0;

static int rstRaw = HIGH, rstStable = HIGH;
static unsigned long rstChange = 0;
static unsigned long resetPressStart = 0;
static bool          resetHolding   = false;
static unsigned long lastResetMsg   = 0;

void ButtonManager::begin(AppConfig& cfg) {
  g_cfg = &cfg;
  pinMode(BTN_CYCLE_PIN, INPUT_PULLUP);
  pinMode(BTN_RESET_PIN, INPUT_PULLUP);
}

void ButtonManager::loop() {
  unsigned long now = millis();

  // ---- Nut cuon (debounce on dinh) ----
  int r1 = digitalRead(BTN_CYCLE_PIN);
  if (r1 != cycRaw) { cycRaw = r1; cycChange = now; }
  if (now - cycChange > BTN_DEBOUNCE_MS && cycStable != cycRaw) {
    cycStable = cycRaw;
    if (cycStable == LOW) cyclePending = true;   // chi tinh 1 lan khi on dinh xuong thap
  }

  // ---- Nut reset (debounce on dinh) ----
  int r2 = digitalRead(BTN_RESET_PIN);
  if (r2 != rstRaw) { rstRaw = r2; rstChange = now; }
  if (now - rstChange > BTN_DEBOUNCE_MS && rstStable != rstRaw) {
    rstStable = rstRaw;
  }

  if (rstStable == LOW) {
    if (resetPressStart == 0) resetPressStart = now;
    resetHolding = true;
    unsigned long held = now - resetPressStart;

    if (now - lastResetMsg > 250) {
      lastResetMsg = now;
      char l2[24];
      if (held >= RESET_FACTORY_MS) {
        DisplayManager::showMessage(T("THẢ RA", "RELEASE"), T("= RESET TẤT CẢ", "= RESET ALL"));
      } else if (held >= RESET_CLEAR_MS) {
        snprintf(l2, sizeof(l2), "30s=Reset (%lus)", (RESET_FACTORY_MS - held) / 1000);
        DisplayManager::showMessage(T("Thả = Xoá WiFi", "Release=Clear WiFi"), l2);
      } else {
        snprintf(l2, sizeof(l2), T("Giữ 5s (%lus)", "Hold 5s (%lus)"), (RESET_CLEAR_MS - held) / 1000 + 1);
        DisplayManager::showMessage(T("Đang giữ nút", "Holding..."), l2);
      }
    }
  } else {
    if (resetPressStart != 0) {
      unsigned long held = now - resetPressStart;
      resetPressStart = 0;
      resetHolding = false;

      if (held >= RESET_FACTORY_MS) {
        DisplayManager::showMessage(T("RESET TẤT CẢ", "RESET ALL"), T("Khởi động lại...", "Restarting..."));
        delay(800);
        Storage::clearAll();
        ESP.restart();
      } else if (held >= RESET_CLEAR_MS) {
        DisplayManager::showMessage(T("Đã xoá WiFi", "WiFi cleared"), T("Về chế độ AP...", "Back to AP..."));
        g_cfg->wifi_ssid[0] = '\0';
        g_cfg->wifi_pass[0] = '\0';
        Storage::save(*g_cfg);
        delay(800);
        ESP.restart();
      }
    }
  }
}

bool ButtonManager::consumeCyclePress() {
  if (cyclePending) { cyclePending = false; return true; }
  return false;
}

bool ButtonManager::isResetHolding() { return resetHolding; }
