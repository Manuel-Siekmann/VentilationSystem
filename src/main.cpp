/*
  This file is part of the SEVentilation to MQTT project.
  Copyright (C) 2023 Dr. Manuel Siekmann. All rights reserved.
*/

#ifdef ESP32
#include <WiFi.h>
#define SET_HOSTNAME WiFi.setHostname
#else
#include <ESP8266WiFi.h>
#define SET_HOSTNAME WiFi.hostname
#endif

#include "SEController.h"
#include "MqttBridge.h"
#include "Logging.h"
#include "WebInterface.h"
#include "env_config.h" // see env_config.h.example


SEController *SEC;
MqttBridge *MQTT;
// WebInterface *WebUI;

void setup()
{
    Serial.begin(115200);
    SET_HOSTNAME(HOSTNAME);
    Log("Attempting to connect to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(500);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Log("Could not connect to WiFi. Status: " + String(WiFi.status()));
        Log("Attempting to connect to WiFi...");
    }

    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    Log("---- Setup: WiFi connected ----");

    SEC = new SEController(RX_PIN, TX_PIN);
    // MQTT = new MqttBridge(MQTT_HOST, MQTT_PORT, SEC);
    WebUI = new WebInterface(SEC);
    WebUI->begin();
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
    MQTT->Poll(); 
    // WebUI->loop();
}
