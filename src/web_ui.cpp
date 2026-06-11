#include "web_ui.h"
#include "config_store.h"

String WebUi::htmlIndex() {
  return R"HTML(
<!doctype html>
<html>
<head>
  <meta name='viewport' content='width=device-width,initial-scale=1'>
  <title>WAVIN Floor Heating</title>
  <style>
    body{font-family:Arial,sans-serif;margin:16px;background:#f4f6f8;color:#111}
    .card{background:#fff;border-radius:10px;padding:12px;margin:10px 0;box-shadow:0 2px 8px rgba(0,0,0,.08)}
    button,input,select{font-size:16px;padding:8px}
    .zone{display:grid;grid-template-columns:1fr auto;gap:8px;align-items:center}
  </style>
</head>
<body>
  <h2>WAVIN Floor Heating</h2>
  <div class='card'>
    <div id='ctrl'></div>
    <button onclick='factoryReset()' style='background:#b00020;color:#fff;border:none;border-radius:6px'>Factory reset</button>
  </div>
  <div id='zones'></div>
<script>
async function load(){
  const r = await fetch('/api/status');
  const s = await r.json();
  document.getElementById('ctrl').innerText = `HW: ${s.controller.hwModel} | FW: ${s.controller.fwVersion} | SN: ${s.controller.serialNo}`;
  const z = document.getElementById('zones'); z.innerHTML='';
  s.zones.forEach((x,i)=>{
    const d = document.createElement('div'); d.className='card zone';
    d.innerHTML = `<div><b>${x.roomName}</b><br>Current: ${x.currentTempC}°C | Target: ${x.targetTempC}°C | Battery: ${x.batteryPct}%</div>
    <div><input id='sp${i}' type='number' min='6' max='40' step='0.5' value='${x.targetTempC}'><button onclick='setp(${i})'>Set</button></div>`;
    z.appendChild(d);
  });
}
async function setp(i){
  const temp = parseFloat(document.getElementById('sp'+i).value);
  await fetch('/api/setpoint',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({zone:i,tempC:temp})});
  load();
}
async function factoryReset(){
  if(!confirm('Factory reset module?')) return;
  await fetch('/api/factory-reset',{method:'POST'});
  alert('Resetting now... reconnect to WiFiManager AP');
}
load(); setInterval(load, 5000);
</script>
</body>
</html>)HTML";
}

void WebUi::begin(AppConfig* cfg, ZoneState zones[MAX_ZONES], ControllerInfo* info, WavinModbus* modbus) {
  cfg_ = cfg;
  zones_ = zones;
  info_ = info;
  modbus_ = modbus;

  server.on("/", HTTP_GET, [this](AsyncWebServerRequest* req) {
    req->send(200, "text/html", htmlIndex());
  });

  server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* req) {
    DynamicJsonDocument doc(4096);
    JsonObject c = doc.createNestedObject("controller");
    c["hwModel"] = info_->hwModel;
    c["fwVersion"] = info_->fwVersion;
    c["serialNo"] = info_->serialNo;

    JsonArray zs = doc.createNestedArray("zones");
    for (int i = 0; i < MAX_ZONES; i++) {
      JsonObject z = zs.createNestedObject();
      z["roomName"] = zones_[i].roomName;
      z["currentTempC"] = isnan(zones_[i].currentTempC) ? 0 : zones_[i].currentTempC;
      z["targetTempC"] = isnan(zones_[i].targetTempC) ? 0 : zones_[i].targetTempC;
      z["batteryPct"] = zones_[i].batteryPct;
    }

    String out;
    serializeJson(doc, out);
    req->send(200, "application/json", out);
  });

  AsyncCallbackJsonWebHandler* setpoint = new AsyncCallbackJsonWebHandler("/api/setpoint", [this](AsyncWebServerRequest* req, JsonVariant& json) {
    JsonObject obj = json.as<JsonObject>();
    int zone = obj["zone"] | -1;
    float t = obj["tempC"] | -100.0f;
    if (zone < 0 || zone >= MAX_ZONES) {
      req->send(400, "application/json", "{\"ok\":false,\"error\":\"invalid zone\"}");
      return;
    }
    if (t < MIN_SETPOINT_C || t > MAX_SETPOINT_C) {
      req->send(400, "application/json", "{\"ok\":false,\"error\":\"temp out of range\"}");
      return;
    }
    bool ok = modbus_->setZoneTarget((uint8_t)zone, t);
    req->send(ok ? 200 : 500, "application/json", ok ? "{\"ok\":true}" : "{\"ok\":false}");
  });
  server.addHandler(setpoint);

  server.on("/api/factory-reset", HTTP_POST, [](AsyncWebServerRequest* req) {
    factoryResetConfig();
    req->send(200, "application/json", "{\"ok\":true}");
    delay(300);
    ESP.restart();
  });

  server.begin();
}
