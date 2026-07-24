#include "ServoController.h"

namespace {
// CR test sequence: center -> max -> center -> min -> center.
// Holding at center twice lets you confirm a continuous-rotation servo
// actually stops both times, not just coincidentally once.
struct CrStage { int us; const char* label; };
const CrStage kCrStages[] = {
  {SERVO_US_CENTER, "CENTER \xe2\x80\x94 should be still / stopped"},
  {SERVO_US_MAX,    "MAX \xe2\x80\x94 spins continuously, or moves & holds a position?"},
  {SERVO_US_CENTER, "CENTER \xe2\x80\x94 should stop again if continuous"},
  {SERVO_US_MIN,    "MIN \xe2\x80\x94 watch direction / behaviour"},
  {SERVO_US_CENTER, "CENTER \xe2\x80\x94 test complete"},
};
const int kCrStageCount = sizeof(kCrStages) / sizeof(kCrStages[0]);
const unsigned long kCrStepMs = 2500;
const unsigned long kSweepStepMs = 15;
const int kSweepStepUs = 5;
}  // namespace

void ServoController::begin() {
  pinMode(SERVO_POT_PIN, INPUT);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  ledcSetup(SERVO_PWM_CHANNEL, SERVO_PWM_FREQ_HZ, SERVO_PWM_RES_BITS);
  ledcAttachPin(SERVO_PWM_PIN, SERVO_PWM_CHANNEL);
  // ESP32 Arduino core 3.x equivalent:
  //   ledcAttach(SERVO_PWM_PIN, SERVO_PWM_FREQ_HZ, SERVO_PWM_RES_BITS);
  //   then ledcWrite(SERVO_PWM_PIN, duty) instead of ledcWrite(SERVO_PWM_CHANNEL, duty)

  writeUs(SERVO_US_CENTER);
  for (int i = 0; i < kAvgSamples; i++) adcBuf_[i] = readPotFiltered();

  sweepUs_ = SERVO_US_MIN;
  sweepDir_ = 1;
}

void ServoController::setExtendedRange(bool enabled) {
  extendedRange_ = enabled;
  if (!enabled) {
    // Snap back into the safe range immediately if we were out in extended territory.
    writeUs(currentUs_);
  }
}

void ServoController::writeUs(int us) {
  int lo = extendedRange_ ? SERVO_US_EXTENDED_MIN : SERVO_US_SAFE_MIN;
  int hi = extendedRange_ ? SERVO_US_EXTENDED_MAX : SERVO_US_SAFE_MAX;
  us = constrain(us, lo, hi);
  // Belt-and-braces: never exceed the hard ceiling regardless of mode.
  us = constrain(us, SERVO_US_HARD_MIN, SERVO_US_HARD_MAX);
  currentUs_ = us;

  uint32_t periodUs = 1000000UL / SERVO_PWM_FREQ_HZ;
  uint32_t maxDuty = (1UL << SERVO_PWM_RES_BITS) - 1;
  uint32_t duty = (uint32_t)((uint64_t)us * maxDuty / periodUs);
  ledcWrite(SERVO_PWM_CHANNEL, duty);
}

int ServoController::percent() const {
  int us = constrain(currentUs_, SERVO_US_MIN, SERVO_US_MAX);
  return map(us, SERVO_US_MIN, SERVO_US_MAX, 0, 100);
}

void ServoController::setMode(ServoMode mode) {
  mode_ = mode;
  crActive_ = false;
  if (mode_ == ServoMode::Sweep) sweepUs_ = currentUs_;
}

const char* ServoController::modeName() const {
  switch (mode_) {
    case ServoMode::Pot:        return "POT";
    case ServoMode::Web:        return "WEB";
    case ServoMode::Sweep:      return "SWEEP";
    case ServoMode::NudgePlus:  return "+50";
    case ServoMode::NudgeMinus: return "-50";
  }
  return "?";
}

void ServoController::nudge(int direction) {
  mode_ = ServoMode::Web;
  crActive_ = false;
  writeUs(currentUs_ + direction * stepUs_);
}

void ServoController::nudgeFixedUs(int deltaUs) {
  // Doesn't touch mode_ - called while already in NudgePlus/NudgeMinus mode.
  crActive_ = false;
  writeUs(currentUs_ + deltaUs);
}

void ServoController::applyPreset(ServoPreset preset) {
  mode_ = ServoMode::Web;
  crActive_ = false;
  switch (preset) {
    case ServoPreset::Min:
      writeUs(SERVO_US_MIN);
      presetCycleIndex_ = 0;
      presetCycleDir_ = 1;  // next physical press heads back toward center
      break;
    case ServoPreset::Max:
      writeUs(SERVO_US_MAX);
      presetCycleIndex_ = 2;
      presetCycleDir_ = -1; // next physical press heads back toward center
      break;
    case ServoPreset::Center:
      writeUs(SERVO_US_CENTER);
      presetCycleIndex_ = 1;
      // direction left as-is - ambiguous which way "center" implies next
      break;
  }
}

void ServoController::cyclePreset() {
  mode_ = ServoMode::Web;
  crActive_ = false;

  // Ping-pong: min -> center -> max -> center -> min -> center -> max ...
  // Direction only flips at the two extremes, so center always sits
  // between a min and a max rather than being skipped over.
  presetCycleIndex_ += presetCycleDir_;
  if (presetCycleIndex_ <= 0) {
    presetCycleIndex_ = 0;
    presetCycleDir_ = 1;
  } else if (presetCycleIndex_ >= 2) {
    presetCycleIndex_ = 2;
    presetCycleDir_ = -1;
  }

  switch (presetCycleIndex_) {
    case 0: writeUs(SERVO_US_MIN);    break;
    case 1: writeUs(SERVO_US_CENTER); break;
    case 2: writeUs(SERVO_US_MAX);    break;
  }
}

void ServoController::startCrTest() {
  mode_ = ServoMode::Web;
  crActive_ = true;
  crStep_ = 0;
  crStepStartMs_ = millis();
  writeUs(kCrStages[0].us);
}

void ServoController::stopCrTest() {
  crActive_ = false;
  writeUs(SERVO_US_CENTER);
}

String ServoController::crTestLabel() const {
  if (!crActive_ || crStep_ >= kCrStageCount) return "";
  return String(kCrStages[crStep_].label);
}

void ServoController::updateCrTest() {
  if (!crActive_) return;
  if (millis() - crStepStartMs_ < kCrStepMs) return;

  crStep_++;
  if (crStep_ >= kCrStageCount) {
    crActive_ = false;
    writeUs(SERVO_US_CENTER);
    return;
  }
  crStepStartMs_ = millis();
  writeUs(kCrStages[crStep_].us);
}

int ServoController::readPotFiltered() {
  int raw = analogRead(SERVO_POT_PIN);
  adcBuf_[adcIdx_] = raw;
  adcIdx_ = (adcIdx_ + 1) % kAvgSamples;
  if (adcIdx_ == 0) adcFilled_ = true;

  int count = adcFilled_ ? kAvgSamples : adcIdx_;
  long sum = 0;
  for (int i = 0; i < count; i++) sum += adcBuf_[i];
  return count > 0 ? (int)(sum / count) : raw;
}

void ServoController::loop() {
  updateCrTest();

  if (crActive_) return; // CR test owns the output while running

  if (mode_ == ServoMode::Pot) {
    int raw = readPotFiltered();
    writeUs(map(raw, 0, 4095, SERVO_US_MIN, SERVO_US_MAX));
  } else if (mode_ == ServoMode::Sweep) {
    if (millis() - lastSweepStepMs_ >= kSweepStepMs) {
      lastSweepStepMs_ = millis();
      sweepUs_ += sweepDir_ * kSweepStepUs;
      if (sweepUs_ >= SERVO_US_MAX) { sweepUs_ = SERVO_US_MAX; sweepDir_ = -1; }
      if (sweepUs_ <= SERVO_US_MIN) { sweepUs_ = SERVO_US_MIN; sweepDir_ = 1; }
      writeUs(sweepUs_);
    }
  }
  // ServoMode::Web is event-driven; WebUi calls writeUs()/nudge()/applyPreset() directly
}
