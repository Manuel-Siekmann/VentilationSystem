/*
  This file is part of the SEVentilation to MQTT project.
  Copyright (C) 2023 Dr. Manuel Siekmann. All rights reserved.
*/

// Documentation for future changes

// Dim screen
// Register: 58 = [minutes]
// Register: 59 = [by xx percent]

// Summer ventilation (no heat recovery)
// Register: 48 = [0A00 = on]; [0800 = off]

// Snooze time
// Register: 56 = [xx minutes]

#ifndef SECONTROLLER_H
#define SECONTROLLER_H

#include <SoftwareSerial.h>
#include <functional>

#define STX 0x02
#define ETX 0x0A
#define ACK 0x06
#define TAB 0x09

#define RESET_ACK_MILLIS 8000

#define PROCESS_REQUESTREGISTER_DELAY_MILLIS 10
#define PROCESS_SENDBUFFER_DELAY_MILLIS 10
#define SEND_ACK_DELAY_MILLIS 2

#define COMMANDID_SET 32
#define COMMANDID_GET 32800

#define SECONTROLLER_BAUD 28800

#define ON_REGISTERCHANGED_MAX 10

class SEController
{
private:
    bool InsideMessageFlag = false;
    bool ReceivedSTXFlag = false;

    bool LastMessageAccepted = true;
    bool SendMessageAck = false;

    unsigned long PreviousMillisProcessFanLevels = 0;
    unsigned long PreviousMillisProcessLabels = 0;
    unsigned long PreviousMillisAckReceived = 0;
    unsigned long PreviousSerialAvailable = 0;

    const unsigned long LABEL_UPDATE_INTERVAL = 600000; // 10 Minuten

    int FanLevelRegisterIndex = 0;
    int LabelRegisterIndex = 0;

    typedef std::function<void(SEController*, int, const char*)> RegisterChangedCallback;
    RegisterChangedCallback OnRegisterChanged[ON_REGISTERCHANGED_MAX];
    unsigned int OnRegisterChangedCount = 0;

    static const int FAN_LEVEL_COUNT = 6;
    static const int FAN_LEVEL_REGISTERS[FAN_LEVEL_COUNT];

    static const int LABEL_COUNT = 6;
    static const int LABEL_REGISTERS[LABEL_COUNT];

    char FanLevelValues[FAN_LEVEL_COUNT][16];
    char LabelValues[LABEL_COUNT][16];

    char SendMessageBuffer[64];
    char ReceiveMessageBuffer[64];

    SoftwareSerial *SECSerial;

    bool IsSendBufferEmpty();
    void SendMessageRequest(int commandId, int registerId);
    void ProcessMessageResponseIncome(int commandId, int registerId, const char* content);
    void ProcessSendMessageAck();
    void ProcessMessage(const char* message);
    void ProcessFanLevelRegisters();
    void ProcessLabelRegisters();
    void ProcessMessageSendBuffer();

    int getFanLevelRegisterIndex(int registerId);
    int getLabelRegisterIndex(int registerId);

public:
    SEController(uint8_t rxPin, uint8_t txPin);
    ~SEController();
    void SendMessageResponse(int registerId, const char* content);
    void AddOnRegisterChanged(RegisterChangedCallback callback);
    void Poll();
};

#endif
