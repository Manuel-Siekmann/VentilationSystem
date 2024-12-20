/*
  This file is part of the SEVentilation to MQTT project.
  Copyright (C) 2023 Dr. Manuel Siekmann. All rights reserved.
*/

#include "WebInterface.h"
#include "Logging.h"
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#define AREA_LEVEL_START 173
#define AREA_LEVEL_END 178
#define LABEL_REGISTER_START 78
#define LABEL_REGISTER_END 83
#define MAX_LEVEL 6

const int NAME_MAPPING_COUNT = 70;
const char* NAME_MAPPING[NAME_MAPPING_COUNT] = {
    "", // 0
    "Bereich 1", // 1
    "Bereich 2", // 2
    "Bereich 3", // 3
    "Bereich 4", // 4
    "Bereich 5", // 5
    "Bereich 6", // 6
    "Wohnzimmer", // 7
    "Wohnzimmer 1", // 8
    "Wohnzimmer 2", // 9
    "Esszimmer", // 10
    "Esszimmer 1", // 11
    "Esszimmer 2", // 12
    "Schlafzimmer", // 13
    "Schlafzimmer 1", // 14
    "Schlafzimmer 2", // 15
    "Kinderzimmer", // 16
    "Kinderzimmer 1", // 17
    "Kinderzimmer 2", // 18
    "Kinderzimmer 3", // 19
    "Kinderzimmer 4", // 20
    "Küche", // 21
    "Küche 1", // 22
    "Küche 2", // 23
    "Bad", // 24
    "Master Bad", // 25
    "Gäste Bad", // 26
    "WC", // 27
    "Gäste WC", // 28
    "Arbeitszimmer", // 29
    "Arbeitszimmer 1", // 30
    "Arbeitszimmer 2", // 31
    "Hobbyraum", // 32
    "Mehrzweckraum", // 33
    "Abstellraum", // 34
    "Kellerraum", // 35
    "Kellerraum 1", // 36
    "Kellerraum 2", // 37
    "Kellerraum 3", // 38
    "Dachboden", // 39
    "Dachboden 1", // 40
    "Dachboden 2", // 41
    "Dachboden 3", // 42
    "Büro", // 43
    "Büro 1", // 44
    "Büro 2", // 45
    "Büro 3", // 46
    "Büro 4", // 47
    "Büro 6", // 48
    "Chef Büro", // 49
    "Abtl.Ltr. Büro", // 50
    "Büro EK", // 51
    "Büro AB", // 52
    "Büro Entw.", // 53
    "Büro Konstr.", // 54
    "Büro Buchh.", // 55
    "Speiseraum", // 56
    "Besp. Raum", // 57
    "Besp. Raum 1", // 58
    "Besp. Raum 2", // 59
    "Besp. Raum 3", // 60
    "Louge", // 61
    "Bibliothek", // 62
    "Fitnessraum", // 63
    "Wintergarten", // 64
    "Bastelraum", // 65
    "Ankleidez.", // 66
    "HWR", // 67
    "leer" // 68
};

WebInterface::WebInterface(SEController* sec) : server(80), SEC(sec) {
    for (int i = 0; i < FAN_COUNT; i++) {
        fanLevels[i] = 0;
        fanLabels[i] = "Lüfter " + String(i + 1);
    }
}

void WebInterface::begin() {
    server.on("/", std::bind(&WebInterface::handleRoot, this));
    server.on("/setlevel", HTTP_POST, std::bind(&WebInterface::handleSetLevel, this));
    server.on("/levels", HTTP_GET, std::bind(&WebInterface::handleGetLevels, this));
    server.on("/restart", HTTP_POST, std::bind(&WebInterface::handleRestart, this));
    server.begin();

    SEC->AddOnRegisterChanged(std::bind(&WebInterface::onRegisterChanged, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void WebInterface::loop() {
    server.handleClient();
}

void WebInterface::handleRoot() {
    String html = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\">";
    html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
    html += "<title>Lüftersteuerung</title>";
    html += "<style>";

    html += "body { font-family: Arial, sans-serif; background-color: #f0f0f0; margin: 0; padding: 20px; }";
    html += "h1 { color: #333; }";
    html += ".container { max-width: 600px; margin: 0 auto; background-color: #fff; padding: 20px; border-radius: 5px; }";
    html += ".fan-control { margin-bottom: 25px; }";
    html += ".fan-label { font-size: 16px; color: #555; margin-bottom: 5px; }";
    html += ".slider { -webkit-appearance: none; width: 100%; height: 15px; border-radius: 5px; background: #d3d3d3; outline: none; opacity: 0.7; transition: opacity .2s; }";
    html += ".slider:hover { opacity: 1; }";
    html += ".slider::-webkit-slider-thumb { -webkit-appearance: none; appearance: none; width: 25px; height: 25px; border-radius: 50%; background: #4CAF50; cursor: pointer; }";
    html += ".slider::-moz-range-thumb { width: 25px; height: 25px; border-radius: 50%; background: #4CAF50; cursor: pointer; }";
    html += ".fan-status { font-size: 14px; color: #777; margin-top: 5px; }";
    html += ".restart-button { margin-top: 20px; padding: 10px 20px; background-color: #f44336; color: #fff; border: none; border-radius: 5px; cursor: pointer; }";
    html += ".restart-button:hover { background-color: #d32f2f; }";
    html += "</style>";
    html += "</head><body>";
    html += "<div class=\"container\">";
    html += "<h1>Lüftersteuerung</h1>";

    html += "<div id=\"fans\"></div>";

    html += "<script>";
    html += "function updateFans() {";
    html += "fetch('/levels').then(response => response.json()).then(data => {";
    html += "let fansDiv = document.getElementById('fans');";
    html += "fansDiv.innerHTML = '';";
    html += "for (let i = 0; i < data.length; i++) {";
    html += "let fan = data[i];";
    html += "let fanDiv = document.createElement('div');";
    html += "fanDiv.className = 'fan-control';";
    html += "let label = document.createElement('div');";
    html += "label.className = 'fan-label';";
    html += "label.innerText = fan.label;";
    html += "fanDiv.appendChild(label);";
    html += "let slider = document.createElement('input');";
    html += "slider.type = 'range';";
    html += "slider.min = '0';";
    html += "slider.max = fan.maxLevel;";
    html += "slider.value = fan.level;";
    html += "slider.className = 'slider';";
    html += "slider.onchange = function() { setLevel(i, this.value); };";
    html += "fanDiv.appendChild(slider);";
    html += "let status = document.createElement('div');";
    html += "status.className = 'fan-status';";
    html += "status.innerHTML = 'Stufe: ' + fan.level;";
    html += "fanDiv.appendChild(status);";
    html += "fansDiv.appendChild(fanDiv);";
    html += "}";
    html += "});";
    html += "}";

    html += "function setLevel(index, level) {";
    html += "fetch('/setlevel', { method: 'POST', headers: { 'Content-Type': 'application/x-www-form-urlencoded' }, body: 'fan=' + index + '&level=' + level }).then(() => {";
    html += "updateFans();";
    html += "});";
    html += "}";

    html += "function restartDevice() {";
    html += "if (confirm('Möchten Sie das Gerät wirklich neu starten?')) {";
    html += "fetch('/restart', { method: 'POST' }).then(() => {";
    html += "alert('Gerät wird neu gestartet...');";
    html += "});";
    html += "}";
    html += "}";

    html += "setInterval(updateFans, 1000);";
    html += "window.onload = function() {";
    html += "updateFans();";
    html += "setTimeout(updateFans, 500);";
    html += "};";
    html += "</script>";

    html += "</div>";
    html += "</body></html>";
    server.send(200, "text/html", html);
}

void WebInterface::handleSetLevel() {
    if (server.hasArg("fan") && server.hasArg("level")) {
        int index = server.arg("fan").toInt();
        int level = server.arg("level").toInt();
        if (index >= 0 && index < FAN_COUNT) {
            if (level >= 0 && level <= MAX_LEVEL) {
                fanLevels[index] = level;

                int registerId = AREA_LEVEL_START + index;
                char valueStr[8];
                snprintf(valueStr, sizeof(valueStr), "%d", level);
                SEC->SendMessageResponse(registerId, valueStr);
            }
        }
    }
    server.send(200, "text/plain", "OK");
}

void WebInterface::handleGetLevels() {
    String json = "[";
    for (int i = 0; i < FAN_COUNT; i++) {
        if (i > 0) json += ",";
        json += "{";
        json += "\"index\":" + String(i) + ",";
        json += "\"level\":" + String(fanLevels[i]) + ",";
        json += "\"maxLevel\":" + String(MAX_LEVEL) + ",";
        json += "\"label\":\"" + escapeJsonString(fanLabels[i]) + "\"";
        json += "}";
    }
    json += "]";
    server.send(200, "application/json", json);
}

void WebInterface::onRegisterChanged(SEController* seController, int registerId, const char* value) {
    if (registerId >= AREA_LEVEL_START && registerId <= AREA_LEVEL_END) {
        int index = registerId - AREA_LEVEL_START;
        if (index >= 0 && index < FAN_COUNT) {
            fanLevels[index] = atoi(value);
        }
    }
    else if (registerId >= LABEL_REGISTER_START && registerId <= LABEL_REGISTER_END) {
        int index = registerId - LABEL_REGISTER_START;
        if (index >= 0 && index < FAN_COUNT) {
            int nameIndex = atoi(value);
            if (nameIndex >= 0 && nameIndex < NAME_MAPPING_COUNT) {
                fanLabels[index] = NAME_MAPPING[nameIndex];
            } else {
                fanLabels[index] = "Unbekannt";
            }
        }
    }
}

void WebInterface::handleRestart() {
    server.send(200, "text/plain", "Gerät wird neu gestartet...");
    delay(100);
    ESP.restart();
}

String WebInterface::escapeJsonString(const String& input) {
    String output = "";
    for (unsigned int i = 0; i < input.length(); i++) {
        char c = input.charAt(i);
        if (c == '\"') {
            output += "\\\"";
        } else if (c == '\\') {
            output += "\\\\";
        } else if (c == '\b') {
            output += "\\b";
        } else if (c == '\f') {
            output += "\\f";
        } else if (c == '\n') {
            output += "\\n";
        } else if (c == '\r') {
            output += "\\r";
        } else if (c == '\t') {
            output += "\\t";
        } else {
            output += c;
        }
    }
    return output;
}
