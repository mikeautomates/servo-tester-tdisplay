#pragma once

#include <WebServer.h>
#include "ServoController.h"
#include "WifiService.h"
#include "PowerMonitor.h"

class WebUi {
public:
  void begin(ServoController* servo, WifiService* wifi, PowerMonitor* power);
  void loop();

private:
  WebServer server_{80};
  ServoController* servo_ = nullptr;
  WifiService* wifi_ = nullptr;
  PowerMonitor* power_ = nullptr;

  void setupRoutes();
  String statusJson() const;
};
