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

  // Top area (mode / us / pct / bar). Cheap to call often - internally
  // skips redraw when nothing has changed.
  void render(const DisplayState& state);

  // Scrolling SSID/IP marquee under the bar. Safe to call every loop() -
  // it throttles and paces its own scroll speed internally.
  void updateMarquee(const String& ssid, const String& ip);

private:
  TFT_eSPI tft_;
  TFT_eSprite marquee_{&tft_};

  String lastKey_;

  String marqueeText_;
  int marqueeTextWidth_ = 0;
  int marqueeX_ = 0;
  bool marqueeFits_ = true;
  unsigned long lastMarqueeStepMs_ = 0;

  static const int kMarqueeY = 104;   // just below the PWM bar
  static const int kMarqueeH = 31;    // remaining height to the bottom of a 135px screen
  static const int kMarqueeW = 240;
  static const int kMarqueeTextSize = 3;
  static const int kMarqueeTopPadPx = 7;   // gap between the PWM bar and the text rather than centering
  static const int kMarqueeStepPx = 3;
  static const unsigned long kMarqueeStepMs = 30;
};
