#include "config_store.h"

static const char* kCfgPath = "/config.json";

bool configBegin() {
  return LittleFS.begin();
}

bool loadConfig(AppConfig& cfg) {
  if (!LittleFS.exists(kCfgPath)) {
    for (int i = 0; i < MAX_ZONES; i++) {
      cfg.zoneNames[i] = String("Zone ") + String(i + 1);
    }
    return true;
  }

  File f = LittleFS.open(kCfgPath, "r");
  if (!f) return false;

  DynamicJsonDocument doc(2048);
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) return false;

  cfg.deviceName = doc["device_name"] | "wavin-floorheat";
  cfg.adminUser = doc["admin_user"] | "admin";
  cfg.adminPass = doc["admin_pass"] | "admin";

  JsonArray zones = doc["zone_names"].as<JsonArray>();
  for (int i = 0; i < MAX_ZONES; i++) {
    if (!zones.isNull() && i < zones.size()) {
      cfg.zoneNames[i] = (const char*)zones[i];
    }
    if (cfg.zoneNames[i].length() == 0) {
      cfg.zoneNames[i] = String("Zone ") + String(i + 1);
    }
  }
  return true;
}

bool saveConfig(const AppConfig& cfg) {
  DynamicJsonDocument doc(2048);
  doc["device_name"] = cfg.deviceName;
  doc["admin_user"] = cfg.adminUser;
  doc["admin_pass"] = cfg.adminPass;

  JsonArray zones = doc.createNestedArray("zone_names");
  for (int i = 0; i < MAX_ZONES; i++) {
    zones.add(cfg.zoneNames[i]);
  }

  File f = LittleFS.open(kCfgPath, "w");
  if (!f) return false;
  if (serializeJsonPretty(doc, f) == 0) {
    f.close();
    return false;
  }
  f.close();
  return true;
}

bool factoryResetConfig() {
  if (LittleFS.exists(kCfgPath)) {
    return LittleFS.remove(kCfgPath);
  }
  return true;
}
