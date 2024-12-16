/*
  This file is part of the SEVentilation to MQTT project.
  Copyright (C) 2023 Dr. Manuel Siekmann. All rights reserved.
*/

#include "SEController.h"
#include "XModemCRC.h"
#include "Logging.h"
#include <string.h>

const int SEController::FAN_LEVEL_REGISTERS[SEController::FAN_LEVEL_COUNT] = {
    173, 174, 175, 176, 177, 178
};

const int SEController::LABEL_REGISTERS[SEController::LABEL_COUNT] = {
    78, 79, 80, 81, 82, 83
};

bool SEController::IsSendBufferEmpty()
{
    return SendMessageBuffer[0] == '\0';
}

void SEController::SendMessageRequest(int commandId, int registerId)
{
    if (!IsSendBufferEmpty()) Log("SendMessageRequest but message buffer != null");

    int len = snprintf(SendMessageBuffer, sizeof(SendMessageBuffer), "%c%d%c%d%c", STX, commandId, TAB, registerId, TAB);

    unsigned short crc = GetXModemCRC(SendMessageBuffer, len);
    len += snprintf(SendMessageBuffer + len, sizeof(SendMessageBuffer) - len, "%u%c", crc, ETX);
}

void SEController::SendMessageResponse(int registerId, const char* content)
{
    if (!IsSendBufferEmpty()) Log("SendMessageResponse but message buffer != null");

    int len = snprintf(SendMessageBuffer, sizeof(SendMessageBuffer), "%c%d%c%d%c%s%c", STX, COMMANDID_SET, TAB, registerId, TAB, content, TAB);

    unsigned short crc = GetXModemCRC(SendMessageBuffer, len);
    len += snprintf(SendMessageBuffer + len, sizeof(SendMessageBuffer) - len, "%u%c", crc, ETX);
}

int SEController::getFanLevelRegisterIndex(int registerId)
{
    for (int i = 0; i < FAN_LEVEL_COUNT; i++)
    {
        if (FAN_LEVEL_REGISTERS[i] == registerId)
        {
            return i;
        }
    }
    return -1;
}

int SEController::getLabelRegisterIndex(int registerId)
{
    for (int i = 0; i < LABEL_COUNT; i++)
    {
        if (LABEL_REGISTERS[i] == registerId)
        {
            return i;
        }
    }
    return -1;
}

void SEController::ProcessMessageResponseIncome(int commandId, int registerId, const char* content)
{
    int index = getFanLevelRegisterIndex(registerId);
    if (index >= 0)
    {
        if (strcmp(FanLevelValues[index], content) != 0)
        {
            Log("Fan value register " + String(registerId) + " changed to " + String(content));
            strncpy(FanLevelValues[index], content, sizeof(FanLevelValues[index]) - 1);
            FanLevelValues[index][sizeof(FanLevelValues[index]) - 1] = '\0';
            for (unsigned int i = 0; i < OnRegisterChangedCount; i++)
            {
                OnRegisterChanged[i](this, registerId, content);
            }
        }
        return;
    }

    index = getLabelRegisterIndex(registerId);
    if (index >= 0)
    {
        if (strcmp(LabelValues[index], content) != 0)
        {
            Log("Fan value register " + String(registerId) + " changed to " + String(content));
            strncpy(LabelValues[index], content, sizeof(LabelValues[index]) - 1);
            LabelValues[index][sizeof(LabelValues[index]) - 1] = '\0';
            for (unsigned int i = 0; i < OnRegisterChangedCount; i++)
            {
                OnRegisterChanged[i](this, registerId, content);
            }
        }
    }
}

void SEController::ProcessSendMessageAck()
{
    if (SendMessageAck && millis() - PreviousSerialAvailable > SEND_ACK_DELAY_MILLIS)
    {
        SendMessageAck = false;
        SECSerial->write(STX);
        SECSerial->write(ACK);
        SECSerial->write(ETX);
    }
}

void SEController::ProcessMessage(const char* message)
{
    if (message[0] == ACK && message[1] == '\0')
    {
        LastMessageAccepted = true;
        PreviousMillisAckReceived = millis();
    }
    else
    {
        SendMessageAck = true;
        int commandId = 0, registerId = 0;
        char content[32] = {0};

        int numScanned = sscanf(message, "%d\t%d\t%31s", &commandId, &registerId, content);

        if (numScanned == 3)
        {
            ProcessMessageResponseIncome(commandId, registerId, content);
        }
        else
        {
            Log("ProcessMessage: sscanf failed");
        }
    }
}

void SEController::ProcessFanLevelRegisters()
{
    if (LastMessageAccepted && IsSendBufferEmpty())
    {
        int registerId = FAN_LEVEL_REGISTERS[FanLevelRegisterIndex];
        SendMessageRequest(COMMANDID_GET, registerId);
        FanLevelRegisterIndex = (FanLevelRegisterIndex + 1) % FAN_LEVEL_COUNT;
        PreviousMillisProcessFanLevels = millis();
    }
}

void SEController::ProcessLabelRegisters()
{
    if (LastMessageAccepted && IsSendBufferEmpty())
    {
        int registerId = LABEL_REGISTERS[LabelRegisterIndex];
        SendMessageRequest(COMMANDID_GET, registerId);
        LabelRegisterIndex++;

        if (LabelRegisterIndex >= LABEL_COUNT)
        {
            LabelRegisterIndex = 0;
            PreviousMillisProcessLabels = millis();
        }
    }
}

void SEController::ProcessMessageSendBuffer()
{
    if (!SendMessageAck && !IsSendBufferEmpty())
    {
        SECSerial->write((uint8_t*)SendMessageBuffer, strlen(SendMessageBuffer));
        SendMessageBuffer[0] = '\0';
        LastMessageAccepted = false;
    }
}

SEController::SEController(uint8_t rxPin, uint8_t txPin)
{
    SECSerial = new SoftwareSerial(rxPin, txPin);
    SECSerial->begin(SECONTROLLER_BAUD);
    SendMessageBuffer[0] = '\0';
    ReceiveMessageBuffer[0] = '\0';

    memset(FanLevelValues, 0, sizeof(FanLevelValues));
    memset(LabelValues, 0, sizeof(LabelValues));

    FanLevelRegisterIndex = 0;
    LabelRegisterIndex = 0;
    LastMessageAccepted = true;
    PreviousMillisProcessFanLevels = millis();
    PreviousMillisProcessLabels = millis() - LABEL_UPDATE_INTERVAL;
}

SEController::~SEController()
{
    delete SECSerial;
    SECSerial = NULL;
}

void SEController::AddOnRegisterChanged(RegisterChangedCallback callback)
{
    if (OnRegisterChangedCount < ON_REGISTERCHANGED_MAX)
    {
        OnRegisterChanged[OnRegisterChangedCount] = callback;
        OnRegisterChangedCount++;
    }
}

void SEController::Poll()
{
    if (SECSerial->available())
    {
        PreviousSerialAvailable = millis();
        char incomingByte = SECSerial->read();

        if (incomingByte == STX)
        {
            InsideMessageFlag = true;
            ReceivedSTXFlag = true;
            ReceiveMessageBuffer[0] = '\0';
        }
        else if (incomingByte == ETX && InsideMessageFlag && ReceivedSTXFlag)
        {
            InsideMessageFlag = false;
            ReceivedSTXFlag = false;
            ProcessMessage(ReceiveMessageBuffer);
        }
        else if (InsideMessageFlag)
        {
            size_t len = strlen(ReceiveMessageBuffer);
            if (len < sizeof(ReceiveMessageBuffer) - 1)
            {
                ReceiveMessageBuffer[len] = incomingByte;
                ReceiveMessageBuffer[len + 1] = '\0';
            }
        }
    }

    unsigned long currentMillis = millis();

    if (currentMillis - PreviousMillisProcessFanLevels > PROCESS_REQUESTREGISTER_DELAY_MILLIS)
    {
        ProcessFanLevelRegisters();
    }

    if ((currentMillis - PreviousMillisProcessLabels >= LABEL_UPDATE_INTERVAL) ||
        (LabelRegisterIndex > 0 && LastMessageAccepted && IsSendBufferEmpty()))
    {
        ProcessLabelRegisters();
    }

    if (currentMillis - PreviousMillisAckReceived > RESET_ACK_MILLIS)
    {
        PreviousMillisAckReceived = currentMillis;
        LastMessageAccepted = true;
    }

    ProcessSendMessageAck();

    if (LastMessageAccepted)
    {
        if (currentMillis - PreviousSerialAvailable > PROCESS_SENDBUFFER_DELAY_MILLIS)
        {
            ProcessMessageSendBuffer();
        }
    }
}
