#pragma once
#include <Arduino.h>
#include "config.h"

//che do hoat dong cua moi relay
enum RelayMode : uint8_t{
    MODE_AUTO = 0,
    MODE_ON = 1,
    MODE_OFF =2
};

struct Schedule {
    char name[24] = "Lich";
    bool enabled = false;
    uint8_t startHour = 0;
    uint8_t startMin = 0;
    uint8_t stopHour =0;
    uint8_t stopMin = 0;
    uint8_t daysMask = 0b1111111;
};

struct RelayConfig{
    char name[24] = "Relay";
    bool activeLow = RELAY_ACTIVE_LOW_DEFAULT;
    RelayMode mode = MODE_AUTO;
    Schedule  schedules[SCHEDULES_PER_RELAY];
};

struct AppConfig{
    char wifi_ssid[33] = "";
    char wifi_pass[65] = "";
    char ntpServer[48] = DEFAULT_NTP_SERVER;
    int32_t gmt_offset = DEFAULT_GMT_OFFSET;
    int32_t dst_offset = DEFAULT_DST_OFFSET;
    char lang[3] = "vi";          // ngon ngu: "vi" hoac "en"

    // Mang / IP
    bool dhcp = true;                       // true = tu dong (DHCP), false = IP tinh
    char ip[16]      = "";
    char gateway[16] = "";
    char subnet[16]  = "255.255.255.0";
    char dns[16]     = "8.8.8.8";
    char mdns[24]    = MDNS_HOST;           // ten mDNS (http://<ten>.local)
    char city[40]    = "";                  // thanh pho thoi tiet (bo trong = tu do qua IP)

    // Tai khoan dang nhap web
    // web_pass: "admin"(mac dinh, thô) hoac chuoi hash 96 ky tu (salt16+hash32 hex) sau khi doi
    char web_user[24] = "admin";
    char web_pass[97] = "admin";

    RelayConfig relays[RELAY_COUNT];
};