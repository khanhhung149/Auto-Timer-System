#include "relayController.h"
#include "config.h"

static bool g_state[RELAY_COUNT];
static bool g_activeLow[RELAY_COUNT];

static void writePhysical(uint8_t ch){
    bool on = g_state[ch];
    digitalWrite(RELAY_PINS[ch], g_activeLow[ch] ? !on: on);
}

void RelayController::begin(const AppConfig& cfg){
    for(uint8_t i =0; i<RELAY_COUNT; i++){
        g_activeLow[i] = cfg.relays[i].activeLow;
        g_state[i] =false;
        
        digitalWrite(RELAY_PINS[i], g_activeLow[i] ? HIGH: LOW);
        pinMode(RELAY_PINS[i], OUTPUT);
        writePhysical(i);
    }
    Serial.println("[Relay] San sang, tat het");
}

void RelayController::set(uint8_t ch, bool on){
    if(ch >= RELAY_COUNT )return;
    g_state[ch] = on;
    writePhysical(ch);
}

bool RelayController::isOn(uint8_t ch) {
    if(ch >= RELAY_COUNT) return false;
    return g_state[ch];
}

void RelayController::setActiveLow(uint8_t ch, bool activeLow){
    if(ch >= RELAY_COUNT) return;
    g_activeLow[ch] = activeLow;
    writePhysical(ch);
}