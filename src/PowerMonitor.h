#pragma once

#include <Arduino.h>
#include "BoardPins.h"

// Reads the supply rail voltage via a resistor divider on VOLTAGE_SENSE_PIN.
// Uses the ESP32's factory-calibrated analogReadMilliVolts() for accuracy,
// smoothed with a moving average since the servo's current draw can put
// noise on the rail.
class PowerMonitor {
public:
  void begin();
  void update(); // call periodically (e.g. every loop()) - internally paced

  // Returns -1.0f (a clear "not available" sentinel) if
  // VOLTAGE_MONITORING_ENABLED is 0 in BoardPins.h.
  float voltageVolts() const;

private:
  static const int kAvgSamples = 16;
  int buf_[kAvgSamples] = {0};
  int idx_ = 0;
  bool filled_ = false;

  float smoothedVoltage_ = 0.0f;
  unsigned long lastSampleMs_ = 0;

  static const unsigned long kSampleIntervalMs = 50;

  int readFilteredMilliVolts();
};
