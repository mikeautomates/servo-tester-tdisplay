#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>

struct DisplayState {
  const char* modeName;
  int currentUs;
  int percent;
  bool crActive;
  String wifiSsid;
  String wifiIp;
};

class DisplayService {
public:
  void begin();
  void render(const DisplayState& state);

private:
  TFT_eSPI tft_;
  String lastKey_;
};
