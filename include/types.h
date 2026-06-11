#pragma once

#include <Arduino.h>

struct ZoneState {
  float currentTempC = NAN;
  float targetTempC = NAN;
  int batteryPct = -1;
  String roomName;
};

struct ControllerInfo {
  String hwModel;
  String fwVersion;
  String serialNo;
};

