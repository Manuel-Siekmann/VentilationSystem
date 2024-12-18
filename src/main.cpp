/*
  This file is part of the SEVentilation to MQTT project.
  Copyright (C) 2023 Dr. Manuel Siekmann. All rights reserved.
*/

#ifdef ESP32
#include <WiFi.h>
#define SET_HOSTNAME WiFi.setHostname
#define PIN_1 GPIO_NUM_4
#define PIN_2 GPIO_NUM_5
#else
#include <ESP8266WiFi.h>
#define SET_HOSTNAME WiFi.hostname
#define PIN_1 D1
#define PIN_2 D2
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
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Log("Attempting to connect to WiFi...");
    }
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    Log("---- Setup: WiFi connected ----");
    
    SEC = new SEController(PIN_1, PIN_2);
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
    MQTT->Poll(); 
    // WebUI->loop();
}
