#include "weathermanager.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

static AppConfig*   g_cfg = nullptr;
static String       city_ = "";       // thanh pho dang dung (override hoac tu do)
static String       temp_ = "";       // "+28°C" hoac "" khi chua co
static unsigned long lastCity = 0;
static unsigned long lastTemp = 0;
static int           cityFails = 0;   // dem that bai de gian thoi gian thu lai
static int           tempFails = 0;

void WeatherManager::begin(AppConfig& cfg) {
  g_cfg = &cfg;
  city_ = ""; temp_ = "";
  lastCity = 0; lastTemp = 0;
}

// Lay "city" tu JSON ipinfo.io: {"ip":"...","city":"Hanoi",...}
static String extractCity(const String& body) {
  int k = body.indexOf("\"city\"");
  if (k < 0) return "";
  int c  = body.indexOf(':', k);
  int q1 = body.indexOf('"', c + 1);
  int q2 = body.indexOf('"', q1 + 1);
  if (q1 < 0 || q2 < 0) return "";
  return body.substring(q1 + 1, q2);
}

// Tu do thanh pho qua public IP
static String detectCity() {
  WiFiClientSecure client; client.setInsecure();
  HTTPClient http; http.setConnectTimeout(3000); http.setTimeout(5000);
  if (!http.begin(client, "https://ipinfo.io/json")) return "";
  http.addHeader("User-Agent", "EETIUM-Timer/1.0");
  String city = "";
  if (http.GET() == 200) { String b = http.getString(); city = extractCity(b); city.trim(); }
  http.end();
  return city;
}

// Thay khoang trang bang %20 (ten city co the co dau cach)
static String urlEnc(const String& s) {
  String o;
  for (uint16_t i = 0; i < s.length(); i++) o += (s[i] == ' ') ? String("%20") : String(s[i]);
  return o;
}

static bool fetchTemp() {
  if (city_.length() == 0) return false;
  WiFiClientSecure client; client.setInsecure();
  HTTPClient http; http.setConnectTimeout(3000); http.setTimeout(5000);
  String url = "https://wttr.in/" + urlEnc(city_) + "?format=%t&m";   // m = do C
  if (!http.begin(client, url)) return false;
  http.addHeader("User-Agent", "curl/8.0");
  bool ok = false;
  if (http.GET() == 200) {
    String b = http.getString(); b.trim();
    if (b.length() > 0 && b.length() < 12 && b.indexOf("Unknown") < 0 && b.indexOf("html") < 0) {
      temp_ = b; ok = true;
    }
  }
  http.end();
  return ok;
}

void WeatherManager::loop() {
  if (WiFi.status() != WL_CONNECTED) return;
  unsigned long now = millis();

  // 1) Xac dinh thanh pho: uu tien override; tu do co BACKOFF khi that bai
  if (city_.length() == 0) {
    if (g_cfg && strlen(g_cfg->city) > 0) {
      city_ = g_cfg->city;
    } else {
      unsigned long wait = (cityFails < 3) ? 15000UL : 600000UL;  // 3 lan dau 15s, sau do 10 phut
      if (lastCity == 0 || now - lastCity > wait) {
        lastCity = now;
        city_ = detectCity();
        if (city_.length() == 0) cityFails++;
      }
    }
    return;
  }
  // Neu override doi -> cap nhat lai
  if (g_cfg && strlen(g_cfg->city) > 0 && city_ != g_cfg->city) {
    city_ = g_cfg->city; temp_ = ""; lastTemp = 0; tempFails = 0;
  }

  // 2) Lay nhiet do: co BACKOFF khi that bai (tranh block lien tuc)
  if (temp_.length() == 0) {
    unsigned long wait = (tempFails < 3) ? 10000UL : 600000UL;
    if (lastTemp == 0 || now - lastTemp > wait) {
      lastTemp = now;
      if (fetchTemp()) tempFails = 0; else tempFails++;
    }
  } else if (now - lastTemp > 1800000UL) {        // 30 phut: lam moi
    lastTemp = now;
    fetchTemp();
  }
}

bool   WeatherManager::hasData() { return temp_.length() > 0; }
String WeatherManager::city()    { return city_; }

String WeatherManager::footer() {
  String t = temp_; t.replace("°", "");           // bo ky tu do (font khong co glyph nay)
  return city_ + " " + t;                          // giu DAU (vd "Hà Nội +26C") - font Viet render duoc
}
