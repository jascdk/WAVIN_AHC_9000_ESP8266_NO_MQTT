#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "config.h"

struct AppConfig {
  String deviceName = "wavin-floorheat";
  String adminUser = "admin";
  String adminPass = "admin";
  String zoneNames[MAX_ZONES];
};

bool configBegin();
bool loadConfig(AppConfig& cfg);
bool saveConfig(const AppConfig& cfg);
bool factoryResetConfig();

