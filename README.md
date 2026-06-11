# WAVIN AHC 9000 ESP8266 (No MQTT)

ESP-01 based local web controller for WAVIN floor heating.

## Features
- WiFiManager onboarding AP (`WAVIN-ESP01-SETUP`)
- Rename thermostat zones in captive portal
- Local web UI on port 80
- Shows battery, current temp, target temp, controller info
- Set target temp 6.0 to 40.0 °C in 0.5 °C steps
- OTA update support
- Factory reset endpoint (`POST /api/factory-reset`)
- Modbus RTU over RS485

## Build
```bash
pio run
```

## Upload
```bash
pio run -t upload
```

## Notes
- Serial settings are configured for Modbus RTU at **38400** and **8E1**.
- ESP-01 uses UART0 for Modbus (`GPIO1 TX`, `GPIO3 RX`) and `GPIO2` for DE/RE direction.
- If your WAVIN register map differs, update constants in `src/modbus_client.cpp`.
