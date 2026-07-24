#include "PowerMonitor.h"

void PowerMonitor::begin() {
#if VOLTAGE_MONITORING_ENABLED
  pinMode(VOLTAGE_SENSE_PIN, INPUT);
  // Seed the averaging buffer so voltageVolts() reads sensibly from the
  // very first call rather than starting at 0.
  for (int i = 0; i < kAvgSamples; i++) buf_[i] = readFilteredMilliVolts();
  filled_ = true;
  update();
#endif
}

int PowerMonitor::readFilteredMilliVolts() {
  // analogReadMilliVolts() uses the ESP32's factory ADC calibration (eFuse
  // data), which is meaningfully more accurate than analogRead() + a manual
  // formula, especially away from the very ends of the ADC's range.
  return analogReadMilliVolts(VOLTAGE_SENSE_PIN);
}

void PowerMonitor::update() {
#if VOLTAGE_MONITORING_ENABLED
  if (millis() - lastSampleMs_ < kSampleIntervalMs) return;
  lastSampleMs_ = millis();

  buf_[idx_] = readFilteredMilliVolts();
  idx_ = (idx_ + 1) % kAvgSamples;
  if (idx_ == 0) filled_ = true;

  int count = filled_ ? kAvgSamples : idx_;
  if (count == 0) return;

  long sum = 0;
  for (int i = 0; i < count; i++) sum += buf_[i];
  float adcMilliVolts = (float)sum / count;

  float dividerRatio = VOLTAGE_DIVIDER_R2_OHMS / (VOLTAGE_DIVIDER_R1_OHMS + VOLTAGE_DIVIDER_R2_OHMS);
  float railVolts = (adcMilliVolts / 1000.0f) / dividerRatio;

  smoothedVoltage_ = railVolts * VOLTAGE_CALIBRATION_FACTOR;
#endif
}

float PowerMonitor::voltageVolts() const {
#if VOLTAGE_MONITORING_ENABLED
  return smoothedVoltage_;
#else
  return -1.0f; // sentinel: monitoring disabled, never wired up
#endif
}
