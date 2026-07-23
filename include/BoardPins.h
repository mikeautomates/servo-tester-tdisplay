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
