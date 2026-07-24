#include "WebUi.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

void WebUi::begin(ServoController* servo, WifiService* wifi, PowerMonitor* power) {
  servo_ = servo;
  wifi_ = wifi;
  power_ = power;
  setupRoutes();
  server_.begin();
}

void WebUi::loop() {
  server_.handleClient();
}

String WebUi::statusJson() const {
  JsonDocument doc;
  doc["us"] = servo_->currentUs();
  doc["pct"] = servo_->percent();
  doc["mode"] = servo_->modeName();
  doc["step"] = servo_->stepUs();
  doc["crActive"] = servo_->crTestActive();
  doc["crLabel"] = servo_->crTestLabel();
  doc["extendedRange"] = servo_->extendedRange();
  doc["rangeMin"] = servo_->rangeMinUs();
  doc["rangeMax"] = servo_->rangeMaxUs();
  doc["wifiSsid"] = wifi_->ssid();
  doc["wifiIp"] = wifi_->ipAddress();
  doc["stationCount"] = wifi_->stationCount();
  float volts = power_->voltageVolts();
  if (volts < 0) {
    doc["voltageV"] = nullptr; // monitoring disabled or not wired up
  } else {
    doc["voltageV"] = roundf(volts * 100) / 100.0f; // 2 decimal places
  }

  String out;
  serializeJson(doc, out);
  return out;
}

void WebUi::setupRoutes() {
  server_.on("/", HTTP_GET, [this]() {
    File f = LittleFS.open("/index.html", "r");
    if (!f) {
      server_.send(500, "text/plain", "index.html missing - did you upload the LittleFS filesystem image?");
      return;
    }
    server_.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server_.sendHeader("Pragma", "no-cache");
    server_.streamFile(f, "text/html");
    f.close();
  });

  server_.on("/favicon.ico", HTTP_GET, [this]() {
    server_.send(404, "text/plain", "Not found");
  });

  server_.on("/api/status", HTTP_GET, [this]() {
    server_.send(200, "application/json", statusJson());
  });

  server_.on("/api/set", HTTP_GET, [this]() {
    if (server_.hasArg("us")) {
      servo_->setMode(ServoMode::Web);
      servo_->writeUs(server_.arg("us").toInt());
    }
    server_.send(200, "application/json", statusJson());
  });

  server_.on("/api/nudge", HTTP_GET, [this]() {
    int dir = server_.hasArg("dir") ? server_.arg("dir").toInt() : 0;
    if (dir != 0) servo_->nudge(dir > 0 ? 1 : -1);
    server_.send(200, "application/json", statusJson());
  });

  server_.on("/api/preset", HTTP_GET, [this]() {
    String type = server_.arg("type");
    if (type == "min") servo_->applyPreset(ServoPreset::Min);
    else if (type == "max") servo_->applyPreset(ServoPreset::Max);
    else servo_->applyPreset(ServoPreset::Center);
    server_.send(200, "application/json", statusJson());
  });

  server_.on("/api/step", HTTP_GET, [this]() {
    if (server_.hasArg("value")) servo_->setStepUs(server_.arg("value").toInt());
    server_.send(200, "application/json", statusJson());
  });

  server_.on("/api/mode", HTTP_GET, [this]() {
    String m = server_.arg("value");
    if (m == "pot") servo_->setMode(ServoMode::Pot);
    else if (m == "sweep") servo_->setMode(ServoMode::Sweep);
    else if (m == "nudgeplus") servo_->setMode(ServoMode::NudgePlus);
    else if (m == "nudgeminus") servo_->setMode(ServoMode::NudgeMinus);
    else servo_->setMode(ServoMode::Web);
    server_.send(200, "application/json", statusJson());
  });

  server_.on("/api/crtest/start", HTTP_GET, [this]() {
    servo_->startCrTest();
    server_.send(200, "application/json", statusJson());
  });

  server_.on("/api/crtest/stop", HTTP_GET, [this]() {
    servo_->stopCrTest();
    server_.send(200, "application/json", statusJson());
  });

  server_.on("/api/extended", HTTP_GET, [this]() {
    if (server_.hasArg("enabled")) {
      servo_->setExtendedRange(server_.arg("enabled") == "1");
    }
    server_.send(200, "application/json", statusJson());
  });
}
