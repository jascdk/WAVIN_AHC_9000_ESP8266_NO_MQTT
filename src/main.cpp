#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <WiFiManager.h>

#include "config.h"
#include "types.h"
#include "config_store.h"
#include "modbus_client.h"
#include "web_ui.h"

AppConfig gCfg;
ZoneState gZones[MAX_ZONES];
ControllerInfo gInfo;
WavinModbus gModbus;
WebUi gWeb;

unsigned long gLastPoll = 0;
bool gShouldSave = false;

void saveCallback() { gShouldSave = true; }

void setupWifiAndPortal() {
  WiFi.mode(WIFI_STA);

  WiFiManager wm;
  wm.setSaveConfigCallback(saveCallback);
  wm.setConfigPortalTimeout(180);

  WiFiManagerParameter pDev("dev", "Device name", gCfg.deviceName.c_str(), 32);
  wm.addParameter(&pDev);

  WiFiManagerParameter* zoneParams[MAX_ZONES];
  char idbuf[16];
  char labelbuf[24];
  static char valbuf[MAX_ZONES][24];
  for (int i = 0; i < MAX_ZONES; i++) {
    snprintf(idbuf, sizeof(idbuf), "z%d", i + 1);
    snprintf(labelbuf, sizeof(labelbuf), "Zone %d name", i + 1);
    strncpy(valbuf[i], gCfg.zoneNames[i].c_str(), sizeof(valbuf[i]) - 1);
    valbuf[i][sizeof(valbuf[i]) - 1] = '\0';
    zoneParams[i] = new WiFiManagerParameter(idbuf, labelbuf, valbuf[i], 24);
    wm.addParameter(zoneParams[i]);
  }

  wm.startConfigPortal("WAVIN-ESP01-SETUP");

  gCfg.deviceName = String(pDev.getValue());
  for (int i = 0; i < MAX_ZONES; i++) {
    gCfg.zoneNames[i] = String(zoneParams[i]->getValue());
    if (gCfg.zoneNames[i].length() == 0) gCfg.zoneNames[i] = String("Zone ") + String(i + 1);
    gZones[i].roomName = gCfg.zoneNames[i];
  }

  saveConfig(gCfg);
}
}

void setupOta() {
  String host = gCfg.deviceName;
  host.replace(" ", "-");
  ArduinoOTA.setHostname(host.c_str());
  ArduinoOTA.begin();
}

void setup() {
  // Keep serial free for Modbus as much as possible
  delay(100);

  configBegin();
  loadConfig(gCfg);

  for (int i = 0; i < MAX_ZONES; i++) {
    gZones[i].roomName = gCfg.zoneNames[i].length() ? gCfg.zoneNames[i] : (String("Zone ") + String(i + 1));
  }

  setupWifiAndPortal();
  setupOta();

  gModbus.begin();
  gWeb.begin(&gCfg, gZones, &gInfo, &gModbus);
}

void loop() {
  ArduinoOTA.handle();
  gWeb.loop();

  unsigned long now = millis();
  if (now - gLastPoll >= POLL_INTERVAL_MS) {
    gModbus.poll(gZones, gInfo);
    gLastPoll = now;
  }
}
