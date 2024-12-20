/*
  This file is part of the SEVentilation to MQTT project.
  Copyright (C) 2023 Dr. Manuel Siekmann. All rights reserved.
*/

#ifndef WEBINTERFACE_H
#define WEBINTERFACE_H

#ifdef ESP32
#include <WebServer.h>
#else
#include <ESP8266WebServer.h>
#endif

#include "SEController.h"

class WebInterface {
private:
#ifdef ESP32
    WebServer server;
#else
    ESP8266WebServer server;
#endif
    SEController* SEC;

    static const int FAN_COUNT = 6;
    int fanLevels[FAN_COUNT];
    String fanLabels[FAN_COUNT];

    void handleRoot();
    void handleSetLevel();
    void handleGetLevels();
    void handleRestart();

    void onRegisterChanged(SEController* seController, int registerId, const char* value);

    String escapeJsonString(const String& input);

public:
    WebInterface(SEController* sec);
    void begin();
    void loop();
};

#endif
