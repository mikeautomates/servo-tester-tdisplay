#pragma once

#include <Arduino.h>
#include "ServoController.h"
#include "DisplayService.h"
#include "WifiService.h"
#include "WebUi.h"
#include "PowerMonitor.h"

class App {
public:
  void begin();
  void loop();

private:
  ServoController servo_;
  DisplayService display_;
  WifiService wifi_;
  WebUi web_;
  PowerMonitor power_;

  // onboard button (GPIO35): short press cycles Pot/Web/Sweep/+50/-50,
  // medium hold centers, very long hold toggles extended range
  bool buttonWasDown_ = false;
  unsigned long buttonPressStartMs_ = 0;
  bool longPressHandled_ = false;
  bool extToggleHandled_ = false;

  // onboard button (GPIO0): cycles Center->Max->Min in Pot/Web/Sweep modes,
  // or steps +/-50us per press while in NudgePlus/NudgeMinus mode.
  // A long press toggles "dual-nudge mode" - see dualNudgeMode_ below.
  bool posButtonWasDown_ = false;
  unsigned long posButtonPressStartMs_ = 0;
  bool posLongPressHandled_ = false;

  // Dual-nudge mode: while active, GPIO0 = -50us, GPIO35 = +50us, both as
  // single presses - a fast two-thumb way to walk the pulse width up/down
  // without cycling through modes. Entered/exited by a long press of GPIO0.
  bool dualNudgeMode_ = false;

  unsigned long lastDisplayMs_ = 0;

  void handleButton();
  void handlePosButton();
  void updateDisplay();
};
