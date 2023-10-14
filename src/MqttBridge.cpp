/*
  This file is part of the VentilationSystem project.
  Copyright (C) 2023 Dr. Manuel Siekmann. All rights reserved.
*/

#include "MqttBridge.h"
#include <ESP8266WiFi.h>
#include <MQTT.h>
#include "Logging.h"

#define ASCII0 48

#define AREA_LEVEL_START 173
#define AREA_LEVEL_END 178

const String AreaListSet[] = {"airsystem/set/area-1", "airsystem/set/area-2", "airsystem/set/area-3", "airsystem/set/area-4", "airsystem/set/area-5", "airsystem/set/area-6"};
const String AreaListState[] = {"airsystem/state/area-1", "airsystem/state/area-2", "airsystem/state/area-3", "airsystem/state/area-4", "airsystem/state/area-5", "airsystem/state/area-6"};

WiFiClient net;
MQTTClient client(256);

MqttBridge::MqttBridge(const char hostname[], int port, SEController *sec)
{
  SEC = sec;
  client.begin(hostname, port, net);
  while (!client.connect("AirSystem")) {
    Log("MQTT connection failed! Retrying...");
    delay(500);
  }

  Log("MqttBridge::MqttBridge connected to MQTT.");
  client.onMessage([this](String &topic, String &payload) {
    Log("Received from MQTT: " + topic + " - " + payload);
    unsigned int index = 0;
    for (String area : AreaListSet)
    {
      if (topic == area)
      {
        int value = payload.toInt();
        if (value > 6) value = 6;
        if (value < 0) value = 0;
        if (this->SEC != NULL) 
        {
          this->SEC->SendMessageResponse(AREA_LEVEL_START + index, String(value));
          Log("Send to SEC Ventilation: " + topic + " - " + payload);
        }
      }
      index++;
    }
  });
  for (String area : AreaListSet)
  {
    client.subscribe(area);
  }
  SEC->AddOnRegisterChanged([this](SEController * seController, int registerId, String value) -> void {
    int index = registerId - AREA_LEVEL_START;
    if (index >= 0 && index < 6) 
    {
      Log("publish new airsystem state to mqtt: " + AreaListState[index] + " - " + value);
      client.publish(AreaListState[index], value);
    }
  });
}

void MqttBridge::Poll()
{
  client.loop();
}
