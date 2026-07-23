#pragma once

#include <Arduino.h>
#include "ServoController.h"
#include "DisplayService.h"
#include "WifiService.h"
#include "WebUi.h"

class App {
public:
  void begin();
  void loop();

private:
  ServoController servo_;
  DisplayService display_;
  WifiService wifi_;
  WebUi web_;

  // onboard button (GPIO35): short press cycles Pot/Web/Sweep/+50/-50,
  // medium hold centers, very long hold toggles extended range
  bool buttonWasDown_ = false;
  unsigned long buttonPressStartMs_ = 0;
  bool longPressHandled_ = false;
  bool extToggleHandled_ = false;

  // onboard button (GPIO0): cycles Center->Max->Min in Pot/Web/Sweep modes,
  // or steps +/-50us per press while in NudgePlus/NudgeMinus mode
  bool posButtonWasDown_ = false;

  unsigned long lastDisplayMs_ = 0;

  void handleButton();
  void handlePosButton();
  void updateDisplay();
};
