#pragma once

#include <Arduino.h>
#include <ModbusMaster.h>
#include "types.h"
#include "config.h"

class WavinModbus {
 public:
  void begin();
  bool poll(ZoneState zones[MAX_ZONES], ControllerInfo& info);
  bool setZoneTarget(uint8_t zoneIndex, float tempC);

 private:
  ModbusMaster node;
  static void preTransmission();
  static void postTransmission();

  bool readHolding(uint16_t addr, uint8_t count);
  bool writeSingle(uint16_t addr, uint16_t value);
};

