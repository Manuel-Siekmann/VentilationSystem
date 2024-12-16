/*
  This file is part of the SEVentilation to MQTT project.
  Copyright (C) 2023 Dr. Manuel Siekmann. All rights reserved.
*/

#ifndef WEBINTERFACE_H
#define WEBINTERFACE_H

#include <ESP8266WebServer.h>
#include "SEController.h"

class WebInterface {
private:
    ESP8266WebServer server;
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
