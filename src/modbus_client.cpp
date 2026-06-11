#include "modbus_client.h"

// ---- Placeholder register map ----
// Adjust these to your exact WAVIN AHC register map from your ESP32 implementation.
static const uint16_t REG_BASE_CUR_TEMP = 0x0100;   // + zone
static const uint16_t REG_BASE_TARGET_TEMP = 0x0200; // + zone
static const uint16_t REG_BASE_BATTERY = 0x0300;    // + zone

static const uint16_t REG_HW_MODEL = 0x0400;
static const uint16_t REG_FW_VERSION = 0x0401;
static const uint16_t REG_SERIAL = 0x0402;

static const uint16_t REG_BASE_SET_TARGET = 0x0200; // + zone

void WavinModbus::preTransmission() {
  digitalWrite(RS485_DE_RE_PIN, HIGH);
}

void WavinModbus::postTransmission() {
  digitalWrite(RS485_DE_RE_PIN, LOW);
}

void WavinModbus::begin() {
  pinMode(RS485_DE_RE_PIN, OUTPUT);
  digitalWrite(RS485_DE_RE_PIN, LOW);

  Serial.begin(MODBUS_BAUDRATE, MODBUS_SERIAL_CONFIG);
  delay(100);

  node.begin(MODBUS_SLAVE_ID, Serial);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
}

bool WavinModbus::readHolding(uint16_t addr, uint8_t count) {
  uint8_t res = node.readHoldingRegisters(addr, count);
  return res == node.ku8MBSuccess;
}

bool WavinModbus::writeSingle(uint16_t addr, uint16_t value) {
  node.setTransmitBuffer(0, value);
  uint8_t res = node.writeMultipleRegisters(addr, 1);
  return res == node.ku8MBSuccess;
}

bool WavinModbus::poll(ZoneState zones[MAX_ZONES], ControllerInfo& info) {
  // Read zones
  for (uint8_t z = 0; z < MAX_ZONES; z++) {
    if (readHolding(REG_BASE_CUR_TEMP + z, 1)) {
      zones[z].currentTempC = node.getResponseBuffer(0) / 10.0f;
    }
    if (readHolding(REG_BASE_TARGET_TEMP + z, 1)) {
      zones[z].targetTempC = node.getResponseBuffer(0) / 10.0f;
    }
    if (readHolding(REG_BASE_BATTERY + z, 1)) {
      zones[z].batteryPct = (int)node.getResponseBuffer(0);
    }
  }

  // Basic controller info
  if (readHolding(REG_HW_MODEL, 1)) info.hwModel = String(node.getResponseBuffer(0));
  if (readHolding(REG_FW_VERSION, 1)) info.fwVersion = String(node.getResponseBuffer(0));
  if (readHolding(REG_SERIAL, 1)) info.serialNo = String(node.getResponseBuffer(0));

  return true;
}

bool WavinModbus::setZoneTarget(uint8_t zoneIndex, float tempC) {
  if (zoneIndex >= MAX_ZONES) return false;
  if (tempC < MIN_SETPOINT_C || tempC > MAX_SETPOINT_C) return false;

  // enforce 0.5 steps
  float stepped = roundf(tempC * 2.0f) / 2.0f;
  uint16_t raw = (uint16_t)roundf(stepped * 10.0f);

  return writeSingle(REG_BASE_SET_TARGET + zoneIndex, raw);
}
