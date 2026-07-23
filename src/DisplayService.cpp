#include "DisplayService.h"

void DisplayService::begin() {
  tft_.init();
  tft_.setRotation(1);
  tft_.fillScreen(TFT_BLACK);

  marquee_.createSprite(kMarqueeW, kMarqueeH);
  marquee_.fillSprite(TFT_BLACK);
  marquee_.setTextSize(kMarqueeTextSize);
  marquee_.setTextColor(TFT_YELLOW, TFT_BLACK);
  marquee_.setTextWrap(false, false);
  marquee_.pushSprite(0, kMarqueeY);
}

void DisplayService::render(const DisplayState& s) {
  // Skip redundant redraws (reduces flicker while the web UI is polling fast)
  String key = String(s.modeName) + ":" + s.currentUs + ":" + s.crActive + ":" + s.extendedRange + ":" + s.dualNudgeActive;
  if (key == lastKey_) return;
  lastKey_ = key;

  // Only clear/redraw the top area - the marquee below manages its own region
  tft_.fillRect(0, 0, 240, kMarqueeY, TFT_BLACK);

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
  tft_.drawRect(barX, barY, barW, barH, s.extendedRange ? TFT_RED : TFT_DARKGREY);
  int fillW = map(constrain(s.currentUs, s.rangeMinUs, s.rangeMaxUs), s.rangeMinUs, s.rangeMaxUs, 0, barW - 2);
  tft_.fillRect(barX + 1, barY + 1, fillW, barH - 2, s.crActive ? TFT_ORANGE : (s.extendedRange ? TFT_RED : TFT_GREEN));

  if (s.extendedRange) {
    tft_.setTextColor(TFT_RED, TFT_BLACK);
    tft_.setTextSize(1);
    tft_.setCursor(barX + barW - 40, barY - 10);
    tft_.print("DANGER");
  }

  if (s.dualNudgeActive) {
    // "+50" for the mode button (GPIO35), top-right corner, near the physical button
    tft_.setTextColor(TFT_GREEN, TFT_BLACK);
    tft_.setTextSize(3);
    int w = tft_.textWidth("+50");
    tft_.setCursor(240 - w - 6, 2);
    tft_.print("+50");
  }
}

void DisplayService::updateMarquee(const String& ssid, const String& ip, bool dualNudgeActive) {
  if (dualNudgeActive) {
    if (dualNudgeLabelDrawn_) return; // static label - draw once, not every call
    marquee_.fillSprite(TFT_BLACK);
    marquee_.setTextSize(3);
    marquee_.setTextColor(TFT_RED, TFT_BLACK);

    // "-50" for the position button (GPIO0), bottom-right corner, near the
    // physical button. If it lands on the wrong side for your board, this
    // is the one line to change - swap it to a left-aligned x instead.
    int w = marquee_.textWidth("-50");
    marquee_.setCursor(kMarqueeW - w - 10, 4); // y=4 keeps the size-3 text within the 31px sprite height
    marquee_.print("-50");

    marquee_.pushSprite(0, kMarqueeY);
    dualNudgeLabelDrawn_ = true;
    return;
  }
  dualNudgeLabelDrawn_ = false; // reset so it redraws fresh the next time this mode is entered

  String text = "WiFi: " + ssid + "     http://" + ip + "     ";

  if (text != marqueeText_) {
    marqueeText_ = text;
    marquee_.setTextSize(kMarqueeTextSize);
    marqueeTextWidth_ = marquee_.textWidth(marqueeText_);
    marqueeFits_ = marqueeTextWidth_ <= kMarqueeW;
    marqueeX_ = marqueeFits_ ? 0 : kMarqueeW;

    // Static case (fits without scrolling): draw once immediately, centered.
    if (marqueeFits_) {
      marquee_.fillSprite(TFT_BLACK);
      int y = kMarqueeTopPadPx;
      marquee_.setCursor((kMarqueeW - marqueeTextWidth_) / 2, y);
      marquee_.print(marqueeText_);
      marquee_.pushSprite(0, kMarqueeY);
    }
  }

  if (marqueeFits_) return; // nothing to animate

  if (millis() - lastMarqueeStepMs_ < kMarqueeStepMs) return;
  lastMarqueeStepMs_ = millis();

  marquee_.fillSprite(TFT_BLACK);
  int y = (kMarqueeH - kMarqueeTextSize * 8) / 2;
  marquee_.setCursor(marqueeX_, y);
  marquee_.print(marqueeText_);
  marquee_.pushSprite(0, kMarqueeY);

  marqueeX_ -= kMarqueeStepPx;
  if (marqueeX_ < -marqueeTextWidth_) marqueeX_ = kMarqueeW;
}
