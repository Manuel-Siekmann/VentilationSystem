/*
  This file is part of the VentilationSystem project.
  Copyright (C) 2023 Dr. Manuel Siekmann. All rights reserved.
*/

#ifndef MQTTBRIDGE_H
#define MQTTBRIDGE_H
#include <MQTT.h>

#include "SEController.h"

class MqttBridge
{
private:

public:
	SEController *SEC;
	MQTTClient *Client;
	MqttBridge();
	MqttBridge(const char hostname[], int port, SEController *sec);
	~MqttBridge();
	void Poll();
};

#endif
