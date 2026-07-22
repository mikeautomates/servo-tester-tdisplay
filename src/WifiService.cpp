#include "WifiService.h"

void WifiService::begin(const String& ssid, const String& password) {
  ssid_ = ssid;
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid_.c_str(), password.length() ? password.c_str() : nullptr);
  delay(200); // let the AP settle before anything reads softAPIP()

  Serial.println("AP started: " + ssid_);
  Serial.println("AP IP: " + ipAddress());
}
