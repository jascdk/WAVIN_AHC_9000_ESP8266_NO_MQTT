#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "types.h"
#include "config_store.h"
#include "modbus_client.h"

class WebUi {
 public:
  void begin(AppConfig* cfg, ZoneState zones[MAX_ZONES], ControllerInfo* info, WavinModbus* modbus);
  void loop() {}

 private:
  AsyncWebServer server{80};
  AppConfig* cfg_ = nullptr;
  ZoneState* zones_ = nullptr;
  ControllerInfo* info_ = nullptr;
  WavinModbus* modbus_ = nullptr;

  String htmlIndex();
};

