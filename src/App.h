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

  // onboard button (GPIO35): short press cycles Pot/Web/Sweep, long press centers
  bool buttonWasDown_ = false;
  unsigned long buttonPressStartMs_ = 0;
  bool longPressHandled_ = false;

  unsigned long lastDisplayMs_ = 0;

  void handleButton();
  void updateDisplay();
};
