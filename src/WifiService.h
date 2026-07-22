#pragma once

#include <Arduino.h>
#include <WiFi.h>

// The servo tester deliberately only runs a SoftAP - no STA/router mode -
// since the main use case is field use with no WiFi infrastructure around
// (e.g. a slope soaring / flying site). Connect your phone straight to it.
class WifiService {
public:
  void begin(const String& ssid, const String& password);
  String ssid() const { return ssid_; }
  String ipAddress() const { return WiFi.softAPIP().toString(); }
  int stationCount() const { return WiFi.softAPgetStationNum(); }

private:
  String ssid_;
};
