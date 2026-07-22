#include "DisplayService.h"

void DisplayService::begin() {
  tft_.init();
  tft_.setRotation(1);
  tft_.fillScreen(TFT_BLACK);
}

void DisplayService::render(const DisplayState& s) {
  // Skip redundant redraws (reduces flicker while the web UI is polling fast)
  String key = String(s.modeName) + ":" + s.currentUs + ":" + s.crActive;
  if (key == lastKey_) return;
  lastKey_ = key;

  tft_.fillScreen(TFT_BLACK);

  tft_.setTextColor(TFT_CYAN, TFT_BLACK);
  tft_.setTextSize(2);
  tft_.setCursor(4, 4);
  tft_.print("MODE: ");
  tft_.print(s.modeName);

  tft_.setTextColor(TFT_WHITE, TFT_BLACK);
  tft_.setTextSize(3);
  tft_.setCursor(4, 30);
  tft_.printf("%4d us", s.currentUs);

  tft_.setTextSize(2);
  tft_.setCursor(4, 62);
  tft_.printf("%3d %%", s.percent);

  int barX = 4, barY = 90, barW = 220, barH = 14;
  tft_.drawRect(barX, barY, barW, barH, TFT_DARKGREY);
  int fillW = map(constrain(s.currentUs, 900, 2100), 900, 2100, 0, barW - 2);
  tft_.fillRect(barX + 1, barY + 1, fillW, barH - 2, s.crActive ? TFT_ORANGE : TFT_GREEN);

  tft_.setTextSize(1);
  tft_.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft_.setCursor(4, 112);
  tft_.print("WiFi: ");
  tft_.print(s.wifiSsid);
  tft_.setCursor(4, 122);
  tft_.print("http://");
  tft_.print(s.wifiIp);

  if (s.crActive) {
    tft_.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft_.setCursor(4, 134);
    tft_.print("CR TEST RUNNING");
  }
}
