#pragma once

#include <WebServer.h>
#include "ServoController.h"
#include "WifiService.h"

class WebUi {
public:
  void begin(ServoController* servo, WifiService* wifi);
  void loop();

private:
  WebServer server_{80};
  ServoController* servo_ = nullptr;
  WifiService* wifi_ = nullptr;

  void setupRoutes();
  String statusJson() const;
};
