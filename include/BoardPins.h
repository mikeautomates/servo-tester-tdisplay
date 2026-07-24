#pragma once

// ── T-Display (ESP32) board pin map for the servo tester ────────────────────
// Mirrors the convention used in mavsphere-sensor-node/include/boards/*.h

#define BOARD_DISPLAY_NAME "T-Display ESP32"

// ── Fixed onboard hardware (DO NOT CHANGE / DO NOT REUSE) ───────────────────
// GPIO  4  = TFT_BL   (backlight PWM)      \
// GPIO  5  = TFT_CS                         |
// GPIO 16  = TFT_DC                         | see User_Setup_TDisplay.h
// GPIO 18  = TFT_SCLK                       |
// GPIO 19  = TFT_MOSI                       |
// GPIO 23  = TFT_RST                       /
#define BOARD_MODE_BUTTON_PIN       35   // onboard button, input-only, mode cycle + long-press center
#define BOARD_POS_BUTTON_PIN         0   // onboard button, position cycle (Center->Max->Min->Center)
                                          // GPIO0 is the boot-strap pin: safe to read as a button once
                                          // running, just don't hold it while power-cycling the board

// ── Servo tester specific pins (free GPIOs on T-Display) ────────────────────
#define SERVO_POT_PIN               34   // potentiometer wiper, ADC1 input-only
#define SERVO_PWM_PIN               27   // servo signal output

// Supply voltage sense (5V/battery rail via a resistor divider - the ESP32's
// ADC can only handle up to ~3.3V, so this MUST go through a divider, never
// straight to the rail). GPIO36 (often silkscreened "VP") is a dedicated
// ADC-only pin with no other use on this board. If it's not broken out on
// your specific T-Display revision, GPIO32 or GPIO33 are free alternatives.
#define VOLTAGE_SENSE_PIN           36

// This feature is fully optional. Set to 0 if you haven't wired the divider
// - GPIO36 has no internal pull resistor, so an unconnected pin floats and
// picks up ambient noise rather than reading a clean 0V. Disabling this
// cleanly hides the feature instead of showing misleading numbers.
#define VOLTAGE_MONITORING_ENABLED   1

// Divider: rail --[R1]--+--[R2]-- GND, ADC reads the R1/R2 junction.
// 220k/100k gives safe headroom up to ~10.5V at the rail before the ADC
// node would approach its 3.3V limit - covers 4xAA/BEC setups comfortably,
// and won't be damaged even if a 2S LiPo (8.4V) ends up wired here by mistake.
#define VOLTAGE_DIVIDER_R1_OHMS  220000.0f  // rail side
#define VOLTAGE_DIVIDER_R2_OHMS  100000.0f  // ground side
// Fine-tune this after comparing a reading against a multimeter - resistor
// tolerance (typically +/-1-5%) is usually the biggest source of error here.
#define VOLTAGE_CALIBRATION_FACTOR   1.0f

// ── PWM (LEDC) channel/timer config ──────────────────────────────────────────
#define SERVO_PWM_CHANNEL            0
#define SERVO_PWM_FREQ_HZ           50
#define SERVO_PWM_RES_BITS          16

// ── Pulse width limits (microseconds) ────────────────────────────────────────
#define SERVO_US_MIN              1000
#define SERVO_US_MAX              2000
#define SERVO_US_CENTER           1500

// Normal ("locked") operating range - safe default for any servo.
#define SERVO_US_SAFE_MIN          900
#define SERVO_US_SAFE_MAX         2100

// Extended range, only reachable when the "Extended Range" toggle is enabled
// on the web UI. Some servos marketed as "180 degree" need pulses out here
// to reach their full mechanical travel - watch/listen for stalling, buzzing,
// or grinding, and back off immediately if you hear any of that.
#define SERVO_US_EXTENDED_MIN       500
#define SERVO_US_EXTENDED_MAX      2500

// Absolute hard ceiling - enforced no matter what, even in extended mode.
// Nothing outside this range is a sensible pulse width for any analog servo.
#define SERVO_US_HARD_MIN           400
#define SERVO_US_HARD_MAX          2600
