#pragma once

#include <Arduino.h>
#include "BoardPins.h"

enum class ServoMode {
  Pot,
  Web,
  Sweep
};

enum class ServoPreset {
  Min,
  Center,
  Max
};

// Owns PWM output, potentiometer reading, sweep mode, and the non-blocking
// continuous-rotation (CR) test sequence. DisplayService and WebUi both read
// from this via getters; only WebUi and App write to it.
class ServoController {
public:
  void begin();
  void loop();

  void setMode(ServoMode mode);
  ServoMode mode() const { return mode_; }
  const char* modeName() const;

  void writeUs(int us);      // clamps to SERVO_US_ABS_MIN/MAX
  int currentUs() const { return currentUs_; }
  int percent() const;       // 0-100 over SERVO_US_MIN..SERVO_US_MAX

  void setStepUs(int stepUs) { stepUs_ = stepUs; }
  int stepUs() const { return stepUs_; }
  void nudge(int direction); // direction: -1 or +1, moves by stepUs_

  void applyPreset(ServoPreset preset);

  void startCrTest();
  void stopCrTest();
  bool crTestActive() const { return crActive_; }
  String crTestLabel() const;

private:
  ServoMode mode_ = ServoMode::Web;
  int currentUs_ = SERVO_US_CENTER;
  int stepUs_ = 10;

  // sweep state
  int sweepUs_ = 0;
  int sweepDir_ = 1;
  unsigned long lastSweepStepMs_ = 0;

  // potentiometer smoothing
  static const int kAvgSamples = 8;
  int adcBuf_[kAvgSamples] = {0};
  int adcIdx_ = 0;
  bool adcFilled_ = false;
  int readPotFiltered();

  // CR test state machine
  bool crActive_ = false;
  int crStep_ = 0;
  unsigned long crStepStartMs_ = 0;
  void updateCrTest();
};
