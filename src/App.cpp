#include "App.h"
#include "BoardPins.h"
#include <LittleFS.h>

namespace {
const char* kApSsid = "ServoTester";
const char* kApPassword = ""; // 8+ chars, or "" for an open network
const unsigned long kLongPressMs = 700;
const unsigned long kDisplayIntervalMs = 150;
}  // namespace

void App::begin() {
  Serial.begin(115200);

  pinMode(BOARD_MODE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(BOARD_POS_BUTTON_PIN, INPUT_PULLUP);

  if (!LittleFS.begin(true)) { // true = format if mount fails (first boot)
    Serial.println("LittleFS mount failed - web UI will not be served");
  }

  servo_.begin();
  display_.begin();
  wifi_.begin(kApSsid, kApPassword);
  web_.begin(&servo_, &wifi_);

  updateDisplay();
}

void App::handleButton() {
  bool down = (digitalRead(BOARD_MODE_BUTTON_PIN) == LOW);

  if (down && !buttonWasDown_) {
    buttonPressStartMs_ = millis();
    longPressHandled_ = false;
  }

  if (down && !longPressHandled_ && (millis() - buttonPressStartMs_ > kLongPressMs)) {
    servo_.applyPreset(ServoPreset::Center);
    longPressHandled_ = true;
  }

  if (!down && buttonWasDown_ && !longPressHandled_) {
    // short press: cycle Pot -> Web -> Sweep -> Pot ...
    switch (servo_.mode()) {
      case ServoMode::Pot:   servo_.setMode(ServoMode::Web);   break;
      case ServoMode::Web:   servo_.setMode(ServoMode::Sweep); break;
      case ServoMode::Sweep: servo_.setMode(ServoMode::Pot);   break;
    }
  }

  buttonWasDown_ = down;
}

void App::handlePosButton() {
  bool down = (digitalRead(BOARD_POS_BUTTON_PIN) == LOW);

  if (!down && posButtonWasDown_) {
    // short press only - no long-press behaviour on this button
    servo_.cyclePreset();
  }

  posButtonWasDown_ = down;
}

void App::updateDisplay() {
  DisplayState state;
  state.modeName = servo_.modeName();
  state.currentUs = servo_.currentUs();
  state.percent = servo_.percent();
  state.crActive = servo_.crTestActive();
  state.wifiSsid = wifi_.ssid();
  state.wifiIp = wifi_.ipAddress();
  display_.render(state);
}

void App::loop() {
  handleButton();
  handlePosButton();
  servo_.loop();
  web_.loop();

  display_.updateMarquee(wifi_.ssid(), wifi_.ipAddress());

  if (millis() - lastDisplayMs_ >= kDisplayIntervalMs) {
    lastDisplayMs_ = millis();
    updateDisplay();
  }
}
