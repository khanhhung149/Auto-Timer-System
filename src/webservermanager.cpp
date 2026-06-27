#include "webservermanager.h"
#include "config.h"
#include "storage.h"
#include "timemanager.h"
#include "relaycontroller.h"
#include "wifimanager.h"
#include "displaymanager.h"
#include "weathermanager.h"
#include <WebServer.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <esp_random.h>
#include <mbedtls/md.h>
#include <mbedtls/pkcs5.h>

// ===== Bao mat mat khau: PBKDF2-HMAC-SHA256 (1000 vong) + salt 16 byte =====
static void bin2hex(const uint8_t* in, size_t n, char* out) {
  static const char* h = "0123456789abcdef";
  for (size_t i = 0; i < n; i++) { out[i*2] = h[in[i] >> 4]; out[i*2+1] = h[in[i] & 0xF]; }
  out[n*2] = 0;
}
static bool hex2bin(const char* in, uint8_t* out, size_t n) {
  auto v = [](char c)->int { if(c>='0'&&c<='9')return c-'0'; if(c>='a'&&c<='f')return c-'a'+10; return -1; };
  for (size_t i = 0; i < n; i++) { int hi=v(in[i*2]), lo=v(in[i*2+1]); if(hi<0||lo<0)return false; out[i]=(hi<<4)|lo; }
  return true;
}
static void pbkdf2(const char* pw, const uint8_t* salt, uint8_t* out32) {
  mbedtls_md_context_t ctx; mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
  mbedtls_pkcs5_pbkdf2_hmac(&ctx, (const unsigned char*)pw, strlen(pw), salt, 16, 1000, 32, out32);
  mbedtls_md_free(&ctx);
}
// Tao chuoi hash 96 ky tu: saltHex(32) + hashHex(64)
static void hashPassword(const char* pw, char* out97) {
  uint8_t salt[16]; for (int i = 0; i < 16; i++) salt[i] = esp_random() & 0xFF;
  uint8_t hash[32]; pbkdf2(pw, salt, hash);
  bin2hex(salt, 16, out97);
  bin2hex(hash, 32, out97 + 32);
}
// Kiem tra mat khau: ho tro mac dinh tho ("admin"), hash 96 ky tu, hoac thô cu (legacy)
static bool checkPassword(const char* typed, const char* stored) {
  size_t n = strlen(stored);
  if (n == 96) {                                   // da bam
    uint8_t salt[16], want[32], got[32];
    if (!hex2bin(stored, salt, 16) || !hex2bin(stored + 32, want, 32)) return false;
    pbkdf2(typed, salt, got);
    uint8_t diff = 0; for (int i = 0; i < 32; i++) diff |= want[i] ^ got[i];  // timing-safe
    return diff == 0;
  }
  if (n == 0) return strcmp(typed, "admin") == 0;  // chua dat -> mac dinh
  return strcmp(typed, stored) == 0;               // thô (mac dinh/legacy)
}

static WebServer  server(WEB_PORT);
static AppConfig* g_cfg = nullptr;
static const char* THU[] = {"CN","T2","T3","T4","T5","T6","T7"};

// ===== Phien dang nhap: token ngau nhien, cho nhieu thiet bi cung luc =====
#define MAX_SESSIONS 5
static char    sessions[MAX_SESSIONS][33];   // token hex(32)+null; "" = slot trong
static uint8_t sessNext = 0;

static void genToken(char* out33) {
  uint8_t b[16]; for (int i = 0; i < 16; i++) b[i] = esp_random() & 0xFF;
  bin2hex(b, 16, out33);                       // 32 ky tu hex ngau nhien
}
static void addSession(char* out33) {
  genToken(out33);
  strlcpy(sessions[sessNext], out33, 33);
  sessNext = (sessNext + 1) % MAX_SESSIONS;     // day -> day phien cu nhat ra
}
static void clearAllSessions() { for (int i = 0; i < MAX_SESSIONS; i++) sessions[i][0] = '\0'; }

// Lay token tu cookie "ESPSESSIONID=<token>"
static String cookieToken() {
  if (!server.hasHeader("Cookie")) return "";
  String c = server.header("Cookie");
  int p = c.indexOf("ESPSESSIONID=");
  if (p < 0) return "";
  p += 13;
  int e = c.indexOf(';', p);
  String t = (e < 0) ? c.substring(p) : c.substring(p, e);
  t.trim();
  return t;
}
static void removeSession(const String& tok) {
  for (int i = 0; i < MAX_SESSIONS; i++) if (tok == sessions[i]) sessions[i][0] = '\0';
}

// ---- Xac thuc: cookie phai khop 1 token dang hoat dong ----
static bool isAuthed() {
  String tok = cookieToken();
  if (tok.length() == 0) return false;
  for (int i = 0; i < MAX_SESSIONS; i++)
    if (strlen(sessions[i]) > 0 && tok == sessions[i]) return true;
  return false;
}
static bool guardApi() {
  if (WiFiManager::isAP() || isAuthed()) return true;
  server.send(401, "application/json", "{\"ok\":false,\"err\":\"auth\"}");
  return false;
}

// Ap lai cau hinh xuong phan cung (sau khi tai/luu)
static void applyConfig() {
  TimeManager::setOffsets(g_cfg->gmt_offset, g_cfg->dst_offset);
  DisplayManager::setLang(g_cfg->lang);   // dong bo ngon ngu xuong OLED
  for (uint8_t i = 0; i < RELAY_COUNT; i++) {
    RelayController::setActiveLow(i, g_cfg->relays[i].activeLow);
  }
}

// ---------- GET /api/status : trang thai thoi gian thuc ----------
static void handleStatus() {
  DateTime t = TimeManager::now();
  JsonDocument doc;
  char buf[40];

  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", t.hour(), t.minute(), t.second());
  doc["time"] = buf;
  snprintf(buf, sizeof(buf), "%s %02d/%02d/%04d",
           THU[t.dayOfTheWeek()], t.day(), t.month(), t.year());
  doc["date"] = buf;
  doc["dow"]  = t.dayOfTheWeek();
  doc["wifi"]    = WiFiManager::isConnected();
  doc["ip"]      = WiFiManager::ip();
  doc["gateway"] = WiFi.gatewayIP().toString();
  doc["subnet"]  = WiFi.subnetMask().toString();
  doc["city"]    = WeatherManager::city();
  doc["rtc"]     = TimeManager::isRtcOk();

  unsigned long s = millis() / 1000;
  snprintf(buf, sizeof(buf), "%lud %02lu:%02lu:%02lu",
           s / 86400, (s % 86400) / 3600, (s % 3600) / 60, s % 60);
  doc["uptime"] = buf;

  JsonArray rel = doc["relays"].to<JsonArray>();
  for (uint8_t i = 0; i < RELAY_COUNT; i++) rel.add(RelayController::isOn(i));

  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

// ---------- GET /api/lang : ngon ngu hien tai (cong khai, cho login/apsetup) ----------
static void handleLang() {
  String out = String("{\"lang\":\"") + g_cfg->lang + "\"}";
  server.send(200, "application/json", out);
}

// ---------- GET /api/config : tra ve toan bo cau hinh (an mat khau) ----------
static void handleGetConfig() {
  if (!guardApi()) return;
  JsonDocument doc;
  Storage::toJson(*g_cfg, doc, false);   // false = bo wifi_pass/web_pass
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

// ---------- POST /api/config : luu cau hinh moi ----------
static void handlePostConfig() {
  if (!guardApi()) return;
  JsonDocument doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "application/json", "{\"ok\":false,\"err\":\"json\"}");
    return;
  }
  Storage::fromJson(*g_cfg, doc);
  Storage::save(*g_cfg);
  applyConfig();
  server.send(200, "application/json", "{\"ok\":true}");
}

// ---------- POST /api/relay : doi che do 1 relay {ch,mode} ----------
static void handleRelay() {
  if (!guardApi()) return;
  JsonDocument doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "application/json", "{\"ok\":false}");
    return;
  }
  int ch   = doc["ch"]   | -1;
  int mode = doc["mode"] | 0;
  if (ch >= 0 && ch < RELAY_COUNT) {
    g_cfg->relays[ch].mode = (RelayMode)mode;
    Storage::save(*g_cfg);
  }
  server.send(200, "application/json", "{\"ok\":true}");
}

// ---------- GET /api/scan : quet cac mang WiFi xung quanh ----------
static void handleScan() {
  int n = WiFi.scanNetworks();
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();
  for (int i = 0; i < n; i++) {
    JsonObject o = arr.add<JsonObject>();
    o["ssid"] = WiFi.SSID(i);
    o["rssi"] = WiFi.RSSI(i);
    o["enc"]  = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
  }
  WiFi.scanDelete();
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

// ---------- POST /api/wifi : luu SSID/mat khau (tu trang apsetup) roi khoi dong lai ----------
static void handleWifiSave() {
  JsonDocument doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "application/json", "{\"ok\":false}");
    return;
  }
  strlcpy(g_cfg->wifi_ssid, doc["ssid"] | "", sizeof(g_cfg->wifi_ssid));
  strlcpy(g_cfg->wifi_pass, doc["pass"] | "", sizeof(g_cfg->wifi_pass));
  Storage::save(*g_cfg);
  server.send(200, "application/json", "{\"ok\":true}");
  delay(600);
  ESP.restart();   // khoi dong lai de ket noi WiFi vua nhap
}

// ---------- POST /api/sync : ep dong bo NTP ngay ----------
static void handleSync() {
  if (!guardApi()) return;
  bool ok = TimeManager::syncfromNTP(g_cfg->ntpServer);
  server.send(200, "application/json", ok ? "{\"ok\":true}" : "{\"ok\":false}");
}

// ---------- POST /api/reset : khoi phuc mac dinh roi khoi dong lai ----------
static void handleReset() {
  if (!guardApi()) return;
  Storage::setDefaults(*g_cfg);
  Storage::save(*g_cfg);
  server.send(200, "application/json", "{\"ok\":true}");
  delay(400);
  ESP.restart();
}

// ---------- Dang nhap / dang xuat / doi mat khau ----------
static void handleLoginGet() {
  File f = LittleFS.open("/login.html", "r");
  if (!f) { server.send(404, "text/plain", "thieu login.html"); return; }
  server.streamFile(f, "text/html");
  f.close();
}
static void handleLoginPost() {
  String u = server.arg("USERNAME");
  String p = server.arg("PASSWORD");
  if (u == g_cfg->web_user && checkPassword(p.c_str(), g_cfg->web_pass)) {
    char tok[33]; addSession(tok);
    server.sendHeader("Set-Cookie", String("ESPSESSIONID=") + tok + "; Path=/");
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  } else {
    server.sendHeader("Location", "/login?e=1");
    server.send(302, "text/plain", "");
  }
}
static void handleLogout() {
  removeSession(cookieToken());                  // huy phien hien tai
  server.sendHeader("Set-Cookie", "ESPSESSIONID=; Path=/; Max-Age=0");
  server.sendHeader("Location", "/login");
  server.send(302, "text/plain", "");
}
static void handlePassword() {
  if (!guardApi()) return;
  JsonDocument doc;
  if (deserializeJson(doc, server.arg("plain"))) { server.send(400, "application/json", "{\"ok\":false}"); return; }
  String oldp = doc["old"] | "";
  String newp = doc["new"] | "";
  if (!checkPassword(oldp.c_str(), g_cfg->web_pass)) { server.send(200, "application/json", "{\"ok\":false,\"err\":\"old\"}"); return; }
  if (newp.length() < 1)       { server.send(200, "application/json", "{\"ok\":false,\"err\":\"empty\"}"); return; }
  char h[100]; hashPassword(newp.c_str(), h);          // luu dang hash, khong luu tho
  strlcpy(g_cfg->web_pass, h, sizeof(g_cfg->web_pass));
  Storage::save(*g_cfg);
  clearAllSessions();                                  // doi mat khau -> dang xuat MOI thiet bi
  server.send(200, "application/json", "{\"ok\":true}");
}

// Phuc vu app chinh (dung chung cho "/" va cac route muc /relay,/wifi,...)
static void serveAppPage() {
  if (WiFiManager::isAP()) {
    File f = LittleFS.open("/apsetup.html", "r");
    if (!f) { server.send(404, "text/plain", "thieu apsetup.html"); return; }
    server.streamFile(f, "text/html");
    f.close();
    return;
  }
  if (!isAuthed()) {
    server.sendHeader("Location", "/login");
    server.send(302, "text/plain", "");
    return;
  }
  File f = LittleFS.open("/index.html", "r");
  if (!f) { server.send(404, "text/plain", "thieu index.html"); return; }
  server.streamFile(f, "text/html");
  f.close();
}

void WebServerManager::begin(AppConfig& cfg) {
  g_cfg = &cfg;

  // Cac route API dang ky TRUOC serveStatic de duoc uu tien
  server.on("/api/status", HTTP_GET,  handleStatus);
  server.on("/api/lang",   HTTP_GET,  handleLang);
  server.on("/api/config", HTTP_GET,  handleGetConfig);
  server.on("/api/config", HTTP_POST, handlePostConfig);
  server.on("/api/relay",  HTTP_POST, handleRelay);
  server.on("/api/scan",   HTTP_GET,  handleScan);
  server.on("/api/wifi",   HTTP_POST, handleWifiSave);
  server.on("/api/sync",   HTTP_POST, handleSync);
  server.on("/api/reset",  HTTP_POST, handleReset);
  server.on("/api/password", HTTP_POST, handlePassword);
  server.on("/login",  HTTP_GET,  handleLoginGet);
  server.on("/login",  HTTP_POST, handleLoginPost);
  server.on("/logout", HTTP_GET,  handleLogout);

  // Trang goc + cac route muc (SPA): deu phuc vu app chinh
  //  (AP -> apsetup; chua login -> /login; da login -> index.html)
  server.on("/",         HTTP_GET, serveAppPage);
  server.on("/relay",    HTTP_GET, serveAppPage);
  server.on("/wifi",     HTTP_GET, serveAppPage);
  server.on("/settings", HTTP_GET, serveAppPage);
  server.on("/system",   HTTP_GET, serveAppPage);

  // Khong co favicon -> tra 204 cho do spam log
  server.on("/favicon.ico", HTTP_GET, []() { server.send(204, "text/plain", ""); });

  // Phuc vu cac file tinh con lai (style.css, app.js, ...) tu LittleFS
  server.serveStatic("/", LittleFS, "/");

  server.onNotFound([]() {
    // Captive portal: khi o AP, dieu huong moi URL la ve trang setup
    // -> dien thoai tu bat trang cau hinh khi vua noi vao AP
    if (WiFiManager::isAP()) {
      server.sendHeader("Location", "http://" + WiFi.softAPIP().toString() + "/", true);
      server.send(302, "text/plain", "");
    } else {
      server.send(404, "text/plain", "Not found");
    }
  });

  // Thu thap header Cookie de kiem tra dang nhap
  const char* headerKeys[] = { "Cookie" };
  server.collectHeaders(headerKeys, 1);

  server.begin();
  Serial.printf("[Web] May chu chay tai cong %d\n", WEB_PORT);
}

void WebServerManager::loop() {
  server.handleClient();
}
