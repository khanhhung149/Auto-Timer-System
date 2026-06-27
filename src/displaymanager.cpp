#include "displaymanager.h"
#include "config.h"
#include <U8g2lib.h>
#include <SPI.h>
#include <qrcode.h>

static U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI u8g2(
    U8G2_R0, OLED_CS_PIN, OLED_DC_PIN, OLED_RST_PIN
);

static const char* THU_VI[] = {"CN","T2","T3","T4","T5","T6","T7"};
static const char* THU_EN[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
static bool useEN = false;   // mac dinh tieng Viet

// Font ho tro tieng Viet co dau (giong he cu) - 16px
#define DISPLAY_VN_FONT u8g2_font_unifont_t_vietnamese1
// TR-style: chon chuoi theo ngon ngu
static inline const char* TR(const char* vi, const char* en){ return useEN ? en : vi; }
// Ve chuoi UTF-8 (co dau) bang font Viet, can giua
static void vnCenter(const char* s, int y){
  u8g2.setFont(DISPLAY_VN_FONT);
  int w = u8g2.getUTF8Width(s);
  u8g2.drawUTF8((128 - w) / 2, y, s);
}
static void vnLeft(const char* s, int x, int y){
  u8g2.setFont(DISPLAY_VN_FONT);
  u8g2.drawUTF8(x, y, s);
}

bool DisplayManager::begin(){
    u8g2.begin();
    u8g2.setContrast(255);
    u8g2.clearBuffer();
    u8g2.sendBuffer();
    return true;
}

void DisplayManager::setLang(const char* lang){
    useEN = (lang && lang[0] == 'e');   // "en" -> tieng Anh
}

bool DisplayManager::isEN(){ return useEN; }

void DisplayManager::showBoot(const char* msg){
    u8g2.clearBuffer();
    vnCenter("Auto Timer", 24);
    vnCenter(msg, 46);
    u8g2.sendBuffer();
}

void DisplayManager::showClock(const DateTime& now){
    char dateBuf[24], timeBuf[12];
    const char* dow = (useEN ? THU_EN : THU_VI)[now.dayOfTheWeek()];
    if (useEN) {
        // EN: Thu MM/DD/YYYY
        snprintf(dateBuf, sizeof(dateBuf), "%s %02d/%02d/%04d",
                 dow, now.month(), now.day(), now.year());
    } else {
        // VI: T5 DD/MM/YYYY
        snprintf(dateBuf, sizeof(dateBuf), "%s %02d/%02d/%04d",
                 dow, now.day(), now.month(), now.year());
    }

    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d",
    now.hour(), now.minute(), now.second());

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_7x14_tr);
    int w = u8g2.getStrWidth(dateBuf);
    u8g2.drawStr((128-w) /2, 14, dateBuf);
    u8g2.drawHLine(0, 20, 128);
    u8g2.setFont(u8g2_font_logisoso24_tn);
    w = u8g2.getStrWidth(timeBuf);
    u8g2.drawStr((128-w)/2, 52, timeBuf);

    u8g2.sendBuffer();
}

// Icon WiFi 16x16 lay tu he thong cham cong cu
#define WIFI_ICON_W 16
#define WIFI_ICON_H 16
static const unsigned char wifi_icon_xbm[] = {
  0x00,0x00, 0x00,0x00, 0xF0,0x0F, 0x1C,0x38, 0x06,0x60, 0xE2,0x47,
  0x38,0x1C, 0x08,0x10, 0x80,0x01, 0x60,0x06, 0x00,0x00, 0x00,0x00,
  0x80,0x01, 0x80,0x01, 0x00,0x00, 0x00,0x00
};

void DisplayManager::showMain(const DateTime& now, bool wifiOn, const char* weather,
                              bool hasNext, uint8_t evRelay, bool evOn,
                              uint8_t evHour, uint8_t evMin){
    const char* dow = (useEN ? THU_EN : THU_VI)[now.dayOfTheWeek()];
    char buf[32];

    u8g2.clearBuffer();

    // Ngay - goc TREN TRAI (kieu he cu)
    if (useEN) snprintf(buf, sizeof(buf), "%s %02d/%02d/%04d", dow, now.month(), now.day(), now.year());
    else       snprintf(buf, sizeof(buf), "%s %02d/%02d/%04d", dow, now.day(), now.month(), now.year());
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 9, buf);

    // Icon WiFi - goc TREN PHAI (chi khi da ket noi)
    if (wifiOn) u8g2.drawXBM(112, 0, WIFI_ICON_W, WIFI_ICON_H, wifi_icon_xbm);

    // Gio lon, can giua
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
    u8g2.setFont(u8g2_font_logisoso24_tn);
    int w = u8g2.getStrWidth(buf);
    u8g2.drawStr((128 - w) / 2, 44, buf);

    // Footer (kieu he cu): uu tien thoi tiet, neu khong co thi su kien ke tiep
    if (weather && weather[0]) {
        snprintf(buf, sizeof(buf), "%s", weather);          // vd "Hanoi +28C"
    } else if (hasNext) {
        const char* act = evOn ? TR("Bật", "ON") : TR("Tắt", "OFF");
        snprintf(buf, sizeof(buf), "-> %02d:%02d R%d %s", evHour, evMin, evRelay + 1, act);
    } else {
        snprintf(buf, sizeof(buf), TR("không có lịch", "no schedule"));
    }
    vnCenter(buf, 61);

    u8g2.sendBuffer();
}

void DisplayManager::showRelays(const bool* states, const char* const* names, uint8_t count){
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x12_tr);
    u8g2.drawStr(0, 11, useEN ? "Relay status" : "Trang thai relay");
    u8g2.drawHLine(0, 14, 128);

    for (uint8_t i = 0; i < count && i < 4; i++) {
        int y = 27 + i * 12;
        char line[28];
        // Cat ten dai con 11 ky tu cho gon 1 dong
        snprintf(line, sizeof(line), "%u %-11.11s %s",
                 i + 1, names[i], states[i] ? "ON" : "off");
        u8g2.drawStr(0, y, line);
    }
    u8g2.sendBuffer();
}

void DisplayManager::showApSetup(const char* ssid, const char* ip){
    u8g2.clearBuffer();
    // Tieu de (font Viet) - ha xuong y17 de DAU khong bi cat tren
    vnCenter(TR("Cấu hình WiFi", "WiFi Setup"), 17);
    // Ten AP - font lon dam cho noi bat
    u8g2.setFont(u8g2_font_7x13B_tr);
    int w = u8g2.getStrWidth(ssid);
    u8g2.drawStr((128 - w) / 2, 39, ssid);
    // IP - font nho can giua (y56 de khong cham day)
    u8g2.setFont(u8g2_font_6x10_tr);
    w = u8g2.getStrWidth(ip);
    u8g2.drawStr((128 - w) / 2, 56, ip);
    u8g2.sendBuffer();
}

void DisplayManager::showMessage(const char* l1, const char* l2){
    u8g2.clearBuffer();
    vnCenter(l1, 28);
    vnCenter(l2, 50);
    u8g2.sendBuffer();
}

void DisplayManager::showQR(const String& url, const String& ipText){
    QRCode qr;
    uint8_t data[qrcode_getBufferSize(2)];
    qrcode_initText(&qr, data, 2, ECC_LOW, url.c_str());

    u8g2.clearBuffer();
    const int scale = 2;
    int qw = qr.size * scale;            // 25*2 = 50
    int xOff = (128 - qw) / 2;           // can giua ngang
    int yOff = 0;
    for (uint8_t y = 0; y < qr.size; y++)
        for (uint8_t x = 0; x < qr.size; x++)
            if (qrcode_getModule(&qr, x, y))
                u8g2.drawBox(xOff + x * scale, yOff + y * scale, scale, scale);

    // IP nam ngay duoi QR, can giua
    u8g2.setFont(u8g2_font_6x10_tf);
    int w = u8g2.getStrWidth(ipText.c_str());
    u8g2.drawStr((128 - w) / 2, 62, ipText.c_str());
    u8g2.sendBuffer();
}

void DisplayManager::showRelaySchedule(uint8_t idx, const RelayConfig& rc){
    char buf[40];
    u8g2.clearBuffer();

    // Ten relay (font Viet, can giua, y16 de dau khong bi cat tren)
    snprintf(buf, sizeof(buf), "R%d %s", idx + 1, rc.name);
    vnCenter(buf, 16);

    // Che do (font Viet, can giua)
    const char* modeStr = (rc.mode == MODE_ON)  ? TR("Bật", "On")
                        : (rc.mode == MODE_OFF) ? TR("Tắt", "Off")
                        :                         TR("Tự động", "Auto");
    snprintf(buf, sizeof(buf), "%s: %s", TR("Chế độ", "Mode"), modeStr);
    vnCenter(buf, 34);

    // 3 lich (font nho 5x8, can giua)
    u8g2.setFont(u8g2_font_5x8_tr);
    for (uint8_t i = 0; i < SCHEDULES_PER_RELAY; i++) {
        const Schedule& s = rc.schedules[i];
        int y = 45 + i * 9;
        if (s.enabled)
            snprintf(buf, sizeof(buf), "L%d  %02d:%02d > %02d:%02d", i + 1,
                     s.startHour, s.startMin, s.stopHour, s.stopMin);
        else
            snprintf(buf, sizeof(buf), "L%d  --:--:--", i + 1);
        int w = u8g2.getStrWidth(buf);
        u8g2.drawStr((128 - w) / 2, y, buf);
    }
    u8g2.sendBuffer();
}
