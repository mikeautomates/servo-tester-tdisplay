# Servo Tester (T-Display ESP32)

A phone-controlled servo tester for the LILYGO TTGO T-Display (ESP32 + 1.14"
ST7789, 135x240). Structured the same way as `mavsphere-sensor-node`
(PlatformIO, per-concern service classes, LittleFS-served web UI).

The ESP32 runs its **own WiFi access point** — no router or internet needed,
so it works fine at a flying field with no infrastructure around. Connect
your phone to the AP and control the servo from a browser.

## Features

- **Potentiometer mode** — physical pot on GPIO34 drives pulse width directly
- **Web mode** — phone UI: radial gauge, slider, manual entry, +/- step
  buttons (1/5/10/25/50µs), Min/Center/Max presets
- **Sweep mode** — auto-sweeps min↔max continuously, useful for exercising a
  servo or listening for binding
- **+50 / -50 modes** (physical buttons only, not shown in the web UI) —
  while in one of these, the position button (GPIO0) nudges the pulse width
  by a fixed 50µs per press instead of cycling presets
- **Dual-nudge mode** (physical buttons only) — a long press (~0.8s) of the
  position button (GPIO0) enters a mode where GPIO0 becomes -50µs and the
  mode button (GPIO35) becomes +50µs, both as ordinary short presses - a
  fast two-thumb way to walk the pulse width up/down without cycling
  through modes first. Another long press of GPIO0 exits back to normal.
  While active, "+50" appears in the top-right corner of the screen (near
  the mode button) and "-50" appears in the bottom-right corner, in place
  of the WiFi/IP marquee for as long as this mode stays active.
- **Continuous-rotation (CR) test** — non-blocking guided sequence
  (Center → Max → Center → Min → Center) with on-screen instructions for
  judging whether a servo is positional or continuous-rotation
- **Extended Range (danger mode)** — an explicit, confirmation-gated toggle
  (web UI, or a ~3s hold of the mode button) that widens the allowed pulse
  width from the safe 900–2100µs default to 500–2500µs, for servos that
  need a wider native range to reach their full mechanical travel. A hard
  400–2600µs ceiling is enforced in firmware regardless of this toggle.
  Always resets to locked on page load / power cycle.
- Onboard button 1 (GPIO35, "mode button"):
  - short press: cycles Pot → Web → Sweep → +50 → -50 → Pot ..., or acts as
    the +50 button while dual-nudge mode is active
  - ~1s hold: snaps straight to center from any mode (a physical "panic
    button" if the servo does something alarming and your phone isn't
    handy) — works regardless of dual-nudge mode
  - ~3s hold: toggles Extended Range on/off — also works regardless of
    dual-nudge mode
- Onboard button 2 (GPIO0, "position button"):
  - short press: cycles Center → Max → Min → Center in Pot/Web/Sweep
    modes, steps the pulse width by a fixed amount in +50/-50 modes, or
    acts as the -50 button while dual-nudge mode is active
  - ~0.8s hold: enters or exits dual-nudge mode
  - GPIO0 is the ESP32's boot-strap pin, but it's only read during normal
    runtime here — safe as a button as long as you don't hold it while
    power-cycling the board.
- SSID/IP shown as a large scrolling marquee under the pulse-width bar so
  it's readable from a few feet away

## Wiring

| Function | Pin |
|---|---|
| Potentiometer wiper | GPIO34 (ADC1, input-only) — outer legs to 3.3V and GND |
| Servo signal | GPIO27 |
| Servo power/ground | **separate 5–6V supply**, GND common with the ESP32 |
| Mode button | GPIO35 (built into the T-Display board) |
| Position button | GPIO0 (built into the T-Display board) |

TFT pins (4, 5, 16, 18, 19, 23) are fixed by the board and already handled
by `include/User_Setup_TDisplay.h` — don't reuse them.

Do not power a servo from the ESP32's 3.3V or USB 5V rail directly; servos
can draw more current than those can safely supply.

## Building & flashing (PlatformIO)

```bash
# build + flash firmware
pio run -e esp32_tdisplay -t upload

# build + upload the web UI (data/index.html) to LittleFS - required once,
# and again any time you edit data/index.html
pio run -e esp32_tdisplay -t uploadfs

# serial monitor
pio device monitor
```

## Using it

1. Power up. The T-Display shows the AP name and IP (default
   `192.168.4.1`).
2. On your phone, join WiFi network **ServoTester**
   (password `servotest123` — both configurable in `src/App.cpp`).
3. Browse to `http://192.168.4.1`.
4. Drag the gauge's slider, type an exact µs value, or use the ± step
   buttons. Tap a preset for instant Min/Center/Max.
5. Tap **Start CR Test** to step through the diagnostic sequence
   automatically; tap Stop any time to abort back to center.

Multiple phones can connect to the AP and will all see live position via
`/api/status`, but only one should be actively driving the servo at a time
to avoid command conflicts.

## Project layout

```
include/
  BoardPins.h              pin map + PWM/pulse-width constants
  User_Setup_TDisplay.h    TFT_eSPI config (reused from mavsphere-sensor-node)
src/
  main.cpp                 entry point
  App.h/.cpp                orchestrator: button handling, display refresh loop
  ServoController.h/.cpp    PWM output, pot reading, sweep, CR test state machine
  DisplayService.h/.cpp     T-Display TFT rendering
  WifiService.h/.cpp        SoftAP setup
  WebUi.h/.cpp              WebServer routes + JSON status API
data/
  index.html                phone-facing web UI (served from LittleFS)
```

## Extending

- To drive the pulse width from an external digital source (e.g. PPM/SBUS
  input, or a serial command from another microcontroller), add a new
  `ServoMode` and feed it from `ServoController::loop()` the same way
  `Pot` and `Sweep` are handled.
- `include/BoardPins.h` documents which GPIOs are free vs fixed if you want
  to add more I/O (e.g. a current-sense resistor on the servo supply, or a
  second servo channel on another free PWM-capable pin).
