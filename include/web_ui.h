#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "types.h"
#include "config_store.h"
#include "modbus_client.h"

class WebUi {
 public:
  void begin(AppConfig* cfg, ZoneState zones[MAX_ZONES], ControllerInfo* info, WavinModbus* modbus);
  void loop();

 private:
  ESP8266WebServer server{80};
  AppConfig* cfg_ = nullptr;
  ZoneState* zones_ = nullptr;
  ControllerInfo* info_ = nullptr;
  WavinModbus* modbus_ = nullptr;

  String htmlIndex();
};
