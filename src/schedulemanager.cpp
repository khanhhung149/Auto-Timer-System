#include "schedulemanager.h"
#include "relayController.h"

static bool scheduleMatches(const Schedule& s, const DateTime& now){
if (!s.enabled) return false;

uint8_t dow = now.dayOfTheWeek();
 if (!(s.daysMask & (1 << dow))) return false;

  int nowMin   = now.hour() * 60 + now.minute();
  int startMin = s.startHour * 60 + s.startMin;
  int stopMin  = s.stopHour  * 60 + s.stopMin;

   if (startMin == stopMin) return false;
if (startMin < stopMin) {
    // Cung ngay: [start, stop)
    return (nowMin >= startMin && nowMin < stopMin);
  } else {
    // Qua dem: [start, 24h) HOAC [0, stop)
    return (nowMin >= startMin || nowMin < stopMin);
  }
}

bool ScheduleManager::shouldBeOn(const RelayConfig& rc, const DateTime& now) {
  // Ep tay thang tat ca
  if (rc.mode == MODE_ON)  return true;
  if (rc.mode == MODE_OFF) return false;

  // MODE_AUTO: bat neu co bat ky lich nao khop (OR)
  for (uint8_t i = 0; i < SCHEDULES_PER_RELAY; i++) {
    if (scheduleMatches(rc.schedules[i], now)) return true;
  }
  return false;
}

// So phut tu bay gio den lan ke tiep gio (h:m) roi vao mot ngay trong daysMask.
static int nextOccMin(const DateTime& now, int h, int m, uint8_t mask) {
  if (mask == 0) return -1;
  int nowDow = now.dayOfTheWeek();
  int nowMod = now.hour() * 60 + now.minute();
  int tgt    = h * 60 + m;
  for (int off = 0; off <= 7; off++) {
    int dow = (nowDow + off) % 7;
    if (mask & (1 << dow)) {
      int delta = off * 1440 + tgt - nowMod;
      if (delta > 0) return delta;   // chi lay moc trong tuong lai
    }
  }
  return -1;
}

ScheduleManager::NextEvent ScheduleManager::nextEvent(const AppConfig& cfg, const DateTime& now) {
  NextEvent best;
  int bestDelta = 1000000;

  for (uint8_t r = 0; r < RELAY_COUNT; r++) {
    const RelayConfig& rc = cfg.relays[r];
    if (rc.mode != MODE_AUTO) continue;   // ep tay khong co su kien lich

    for (uint8_t i = 0; i < SCHEDULES_PER_RELAY; i++) {
      const Schedule& s = rc.schedules[i];
      if (!s.enabled) continue;
      int sm = s.startHour * 60 + s.startMin;
      int em = s.stopHour  * 60 + s.stopMin;
      if (sm == em) continue;

      // Moc BAT
      int dOn = nextOccMin(now, s.startHour, s.startMin, s.daysMask);
      if (dOn > 0 && dOn < bestDelta) {
        bestDelta = dOn;
        best.valid = true; best.relay = r; best.turnOn = true;
        best.hour = s.startHour; best.min = s.startMin;
      }

      // Moc TAT (qua dem -> tat vao ngay hom sau: dich mask 1 ngay)
      uint8_t offMask = s.daysMask;
      if (sm > em) offMask = ((s.daysMask << 1) | (s.daysMask >> 6)) & 0x7F;
      int dOff = nextOccMin(now, s.stopHour, s.stopMin, offMask);
      if (dOff > 0 && dOff < bestDelta) {
        bestDelta = dOff;
        best.valid = true; best.relay = r; best.turnOn = false;
        best.hour = s.stopHour; best.min = s.stopMin;
      }
    }
  }
  if (best.valid) best.inMinutes = (uint16_t)bestDelta;
  return best;
}

void ScheduleManager::update(const AppConfig& cfg, const DateTime& now) {
  for (uint8_t ch = 0; ch < RELAY_COUNT; ch++) {
    bool want = shouldBeOn(cfg.relays[ch], now);
    // Chi ghi khi can doi -> tranh "ghi lai" lien tuc
    if (RelayController::isOn(ch) != want) {
      RelayController::set(ch, want);
    }
  }
}