/*
  This file is part of the SEVentilation to MQTT project.
  Copyright (C) 2023 Dr. Manuel Siekmann. All rights reserved.
*/

#include "MqttBridge.h"
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <MQTT.h>
#include "Logging.h"

#define AREA_LEVEL_START 173
#define AREA_LEVEL_END 178

const char* AreaListSet[] = {"airsystem/set/area-1", "airsystem/set/area-2", "airsystem/set/area-3", "airsystem/set/area-4", "airsystem/set/area-5", "airsystem/set/area-6"};
const char* AreaListState[] = {"airsystem/state/area-1", "airsystem/state/area-2", "airsystem/state/area-3", "airsystem/state/area-4", "airsystem/state/area-5", "airsystem/state/area-6"};

WiFiClient net;

MqttBridge::MqttBridge(const char hostname[], int port, SEController *sec) : Client(256)
{
    SEC = sec;
    Client.begin(hostname, port, net);
    ConnectToMQTT();

    Client.onMessage([this](String &topic, String &payload) {
        Log("Received from MQTT: " + topic + " - " + payload);
        for (int index = 0; index < 6; index++)
        {
            if (topic == AreaListSet[index])
            {
                int value = payload.toInt();
                value = max(0, min(value, 6));
                if (this->SEC != NULL)
                {
                    char valueStr[8];
                    snprintf(valueStr, sizeof(valueStr), "%d", value);
                    this->SEC->SendMessageResponse(AREA_LEVEL_START + index, valueStr);
                    Log("Send to SEC Ventilation: " + topic + " - " + payload);
                }
            }
        }
    });

    for (int index = 0; index < 6; index++)
    {
        Client.subscribe(AreaListSet[index]);
    }

    SEC->AddOnRegisterChanged([this](SEController * seController, int registerId, const char* value) {
        int index = registerId - AREA_LEVEL_START;
        if (index >= 0 && index < 6)
        {
            Log("Publish new airsystem state to MQTT: " + String(AreaListState[index]) + " - " + value);
            Client.publish(AreaListState[index], value);
        }
    });
}

void MqttBridge::Poll()
{
    if (!Client.connected()) {
        Log("MQTT connection lost, attempting to reconnect...");
        ConnectToMQTT();
    }
    Client.loop();
}

MqttBridge::~MqttBridge() 
{
    Client.disconnect();
}

void MqttBridge::ConnectToMQTT()
{
    while (!Client.connect("AirSystem")) {
        Log("MQTT connection failed! Retrying...");
        delay(1000);
    }
    Log("MQTT connected.");
}
