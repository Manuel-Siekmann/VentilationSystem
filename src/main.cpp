/*
  This file is part of the SEVentilation to MQTT project.
  Copyright (C) 2023 Dr. Manuel Siekmann. All rights reserved.
*/

#include <ESP8266WiFi.h>
#include "SEController.h"
#include "MqttBridge.h"
#include "Logging.h"
#include "WebInterface.h"

#define HOSTNAME "HOSTNAME"
#define WIFI_SSID "WIFI_SSID"
#define WIFI_PASSWORD "WIFI_PASSWORD"

#define MQTT_HOST "nodered"
#define MQTT_PORT 1883

SEController *SEC;
MqttBridge *MQTT;
// WebInterface *WebUI;

void setup()
{
    Serial.begin(115200);
    WiFi.hostname(HOSTNAME);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Log("Attempting to connect to WiFi...");
    }
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    Log("---- Setup: WiFi connected ----");

    SEC = new SEController(D1, D2);
    MQTT = new MqttBridge(MQTT_HOST, MQTT_PORT, SEC);
    // WebUI = new WebInterface(SEC);
    // WebUI->begin();
}

void checkWiFiConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        Log("WiFi disconnected, attempting to reconnect...");
        WiFi.reconnect();
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Log("Reconnecting to WiFi...");
        }
        Log("WiFi reconnected.");
    }
}

void loop()
{
    checkWiFiConnection();
    SEC->Poll();
    MQTT->Poll(); // alternative: WebUI->loop();
}
