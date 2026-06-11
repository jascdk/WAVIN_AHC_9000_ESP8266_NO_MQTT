#pragma once

// ===== RS485 / Modbus (aligned with ESP32 setup request) =====
#define MODBUS_BAUDRATE 38400
// SERIAL_8E1 mirrors the Nilan sketch framing style.
#define MODBUS_SERIAL_CONFIG SERIAL_8E1
#define MODBUS_SLAVE_ID 30

// ESP-01 pin constraints:
// UART0: TX=GPIO1, RX=GPIO3 (used for Modbus RTU)
// Direction control pin: choose GPIO2 (safe high at boot)
#define RS485_DE_RE_PIN 2

// ===== Polling =====
#define POLL_INTERVAL_MS 3000

// ===== Heating constraints =====
#define MIN_SETPOINT_C 6.0f
#define MAX_SETPOINT_C 40.0f
#define SETPOINT_STEP_C 0.5f

// ===== Logical zones =====
#define MAX_ZONES 8

