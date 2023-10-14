/*
  This file is part of the VentilationSystem project.
  Copyright (C) 2023 Dr. Manuel Siekmann. All rights reserved.
*/

#include <ESP8266WiFi.h>
#include "SEController.h"
#include "MQTTBridge.h"
#include "Logging.h"

#define HOSTNAME "HOSTNAME"
#define WIFI_SSID "WIFI_SSID"
#define WIFI_PASSWORD "WIFI_PASSWORD"

#define MQTT_HOST "nodered"
#define MQTT_PORT 1883

SEController *SEC;
MqttBridge *MQTT;

void setup()
{
	WiFi.hostname(HOSTNAME);
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
	}
	WiFi.setAutoReconnect(true);
	WiFi.persistent(true);
	Log("---- setup: wifi connected ----");
	SEC = new SEController(D1, D2);
	MQTT = new MqttBridge(MQTT_HOST, MQTT_PORT, SEC);
}

void loop()
{
	SEC->Poll();
	MQTT->Poll();
}
