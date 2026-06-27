#pragma once
#include "datamodel.h"

// Thoi tiet: neu cfg.city co gia tri -> dung; bo trong -> tu do qua IP (ipinfo.io).
// Lay nhiet do tu wttr.in (khong can API key). Cap nhat moi 30 phut.
namespace WeatherManager {
  void   begin(AppConfig& cfg);
  void   loop();
  bool   hasData();
  String footer();   // vd "Hanoi +28C" (da bo dau & ky tu °)
  String city();     // thanh pho dang dung (override hoac tu do)
}
