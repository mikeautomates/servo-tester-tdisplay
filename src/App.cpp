#include "App.h"
#include "BoardPins.h"
#include <LittleFS.h>

namespace {
const char* kApSsid = "ServoTester";
const char* kApPassword = ""; // 8+ chars, or "" for an open network
const unsigned long kLongPressMs = 700;
const unsigned long kVeryLongPressMs = 3000; // holds past this toggle extended range
const unsigned long kDualNudgeLongPressMs = 800; // hold GPIO0 past this to enter/exit dual-nudge
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
  power_.begin();
  wifi_.begin(kApSsid, kApPassword);
  web_.begin(&servo_, &wifi_, &power_);

  updateDisplay();
}

void App::handleButton() {
  bool down = (digitalRead(BOARD_MODE_BUTTON_PIN) == LOW);

  if (down && !buttonWasDown_) {
    buttonPressStartMs_ = millis();
    longPressHandled_ = false;
    extToggleHandled_ = false;
  }

  if (down && !longPressHandled_ && (millis() - buttonPressStartMs_ > kLongPressMs)) {
    servo_.applyPreset(ServoPreset::Center);
    longPressHandled_ = true;
  }

  if (down && !extToggleHandled_ && (millis() - buttonPressStartMs_ > kVeryLongPressMs)) {
    servo_.setExtendedRange(!servo_.extendedRange());
    extToggleHandled_ = true;
  }

  if (!down && buttonWasDown_ && !longPressHandled_) {
    if (dualNudgeMode_) {
      // dual-nudge mode: this button is repurposed as the +50 button
      servo_.nudgeFixedUs(50);
    } else {
      // short press: cycle Pot -> Web -> Sweep -> +50 -> -50 -> Pot ...
      switch (servo_.mode()) {
        case ServoMode::Pot:        servo_.setMode(ServoMode::Web);        break;
        case ServoMode::Web:        servo_.setMode(ServoMode::Sweep);      break;
        case ServoMode::Sweep:      servo_.setMode(ServoMode::NudgePlus);  break;
        case ServoMode::NudgePlus:  servo_.setMode(ServoMode::NudgeMinus); break;
        case ServoMode::NudgeMinus: servo_.setMode(ServoMode::Pot);        break;
      }
    }
  }

  buttonWasDown_ = down;
}

void App::handlePosButton() {
  bool down = (digitalRead(BOARD_POS_BUTTON_PIN) == LOW);

  if (down && !posButtonWasDown_) {
    posButtonPressStartMs_ = millis();
    posLongPressHandled_ = false;
  }

  if (down && !posLongPressHandled_ && (millis() - posButtonPressStartMs_ > kDualNudgeLongPressMs)) {
    dualNudgeMode_ = !dualNudgeMode_;
    if (dualNudgeMode_) servo_.setMode(ServoMode::Web); // freeze Pot/Sweep automation while manually nudging
    posLongPressHandled_ = true;
  }

  if (!down && posButtonWasDown_ && !posLongPressHandled_) {
    // short press
    if (dualNudgeMode_) {
      // dual-nudge mode: this button is repurposed as the -50 button
      servo_.nudgeFixedUs(-50);
    } else {
      switch (servo_.mode()) {
        case ServoMode::NudgePlus:  servo_.nudgeFixedUs(50);  break;
        case ServoMode::NudgeMinus: servo_.nudgeFixedUs(-50); break;
        default:                    servo_.cyclePreset();     break;
      }
    }
  }

  posButtonWasDown_ = down;
}

void App::updateDisplay() {
  DisplayState state;
  state.modeName = dualNudgeMode_ ? "DUAL" : servo_.modeName(); // short - "+50" now sits top-right on this row
  state.currentUs = servo_.currentUs();
  state.percent = servo_.percent();
  state.crActive = servo_.crTestActive();
  state.extendedRange = servo_.extendedRange();
  state.dualNudgeActive = dualNudgeMode_;
  state.rangeMinUs = servo_.rangeMinUs();
  state.rangeMaxUs = servo_.rangeMaxUs();
  state.wifiSsid = wifi_.ssid();
  state.wifiIp = wifi_.ipAddress();
  state.supplyVoltage = power_.voltageVolts();
  display_.render(state);
}

void App::loop() {
  handleButton();
  handlePosButton();
  servo_.loop();
  web_.loop();
  power_.update();

  display_.updateMarquee(wifi_.ssid(), wifi_.ipAddress(), dualNudgeMode_, power_.voltageVolts());

  if (millis() - lastDisplayMs_ >= kDisplayIntervalMs) {
    lastDisplayMs_ = millis();
    updateDisplay();
  }
}
