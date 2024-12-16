/*
  This file is part of the SEVentilation to MQTT project.
  Copyright (C) 2023 Dr. Manuel Siekmann. All rights reserved.
*/

#ifndef MQTTBRIDGE_H
#define MQTTBRIDGE_H
#include <MQTT.h>
#include "SEController.h"

class MqttBridge
{
private:
    void ConnectToMQTT();
    SEController *SEC;
    MQTTClient Client;

public:
    MqttBridge(const char hostname[], int port, SEController *sec);
    ~MqttBridge();
    void Poll();
};

#endif
