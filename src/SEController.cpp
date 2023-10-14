/*
  This file is part of the VentilationSystem project.
  Copyright (C) 2023 Dr. Manuel Siekmann. All rights reserved.
*/

#include "SEController.h"
#include "XModemCRC.h"
#include "Logging.h"

#define STX 0x02
#define ETX 0x0A
#define ACK 0x06
#define TAB 0x09

#define RESET_ACK_MILLIS 8000

#define PROCESS_REQUESTREGISTER_DELAY_MILLIS 50
#define PROCESS_SENDBUFFER_DELAY_MILLIS 20
#define SEND_ACK_DELAY_MILLIS 2

#define COMMANDID_SET 32
#define COMMANDID_GET 32800

#define SECONTROLLER_BAUD 28800

#define AREA_LEVEL_START 173
#define AREA_LEVEL_END 178

#define REGISTER_MAX 256

#define EMPTY_STRING ""

bool SEController::IsSendBufferEmpty()
{
  return SendMessageBuffer.length() == 0;
}

void SEController::SendMessageRequest(int commandId, int registerId)
{
  if (IsSendBufferEmpty() == false) Log("SendMessageRequest but message buffer != null");
  String message = char(STX) + String(commandId) + char(TAB) + String(registerId) + char(TAB);
  SendMessageBuffer = message + GetXModemCRC(message) + char(ETX);
}

void SEController::SendMessageResponse(int registerId, String content)
{
  if (IsSendBufferEmpty() == false) Log("SendMessageResponse but message buffer != null");
  String message = char(STX) + String(COMMANDID_SET) + char(TAB) + String(registerId) + char(TAB) + content + char(TAB);
  SendMessageBuffer = message + GetXModemCRC(message) + char(ETX);
}

void SEController::ProcessMessageResponseIncome(int commandId, int registerId, String content)
{
  if (Register[registerId] != content)
  {
    Log("ProcessMessageResponseIncome: register changed");
    Register[registerId] = content;
    if (AREA_LEVEL_START <= registerId && registerId <= AREA_LEVEL_END)
    {
      for (unsigned int index = 0; index < OnRegisterChangedCount; index++)
      {
        OnRegisterChanged[index](this, registerId, content);
      }
    }
  }
}

void SEController::ProcessSendMessageAck()
{
  if (SendMessageAck && millis() - PreviousSerialAvailable > SEND_ACK_DELAY_MILLIS)
  {
    SendMessageAck = false;
    SECSerial->print(char(STX));
    SECSerial->print(char(ACK));
    SECSerial->print(char(ETX));
  }
}

void SEController::ProcessMessage(String message)
{
  unsigned int MAX_PARTS = 5;
  const char delimiter = TAB;
  unsigned int numPart = 0;
  String parts[MAX_PARTS];
  unsigned int startIndex = 0;
  if (message == String(char(ACK)))
  {
    LastMessageAccepted = true;
    PreviousMillisAckReceived = millis();
  }
  else
  {
    SendMessageAck = true;
    for (uint i = 0; i < message.length(); i++)
    {
      if (message.charAt(i) == delimiter)
      {
        parts[numPart] = message.substring(startIndex, i);
        numPart++;
        startIndex = i + 1;
      }
      if (numPart == MAX_PARTS - 1)
        break;
    }
    parts[numPart] = message.substring(startIndex);
    numPart++;
  }
  if (numPart == 4)
  {
    ProcessMessageResponseIncome(parts[0].toInt(), parts[1].toInt(), parts[2]);
  }
}

void SEController::ProcessRequestNextRegister()
{
  if (LastMessageAccepted && IsSendBufferEmpty())
  {
    SendMessageRequest(COMMANDID_GET, RegisterIndex);
    RegisterIndex++;
    PreviousMillisProcessRegister = millis();
    if (RegisterIndex > AREA_LEVEL_END)
      RegisterIndex = AREA_LEVEL_START;
  }
}

void SEController::ProcessMessageSendBuffer()
{
  if (SendMessageAck == false && SendMessageBuffer.length() > 0)
  {
    SECSerial->print(SendMessageBuffer);
    SendMessageBuffer = EMPTY_STRING;
    LastMessageAccepted = false;
  }
}

SEController::SEController(uint8_t rxPin, uint8_t txPin)
{
  SECSerial = new SoftwareSerial(rxPin, txPin);
  SECSerial->begin(SECONTROLLER_BAUD);
}

SEController::~SEController()
{
  delete SECSerial;
  SECSerial = NULL;
}

void SEController::AddOnRegisterChanged(RegisterChangedCallback callback)
{
  OnRegisterChanged[OnRegisterChangedCount] = callback;
  OnRegisterChangedCount++;
}

void SEController::Poll()
{
  char incomingByte;
  if (SECSerial->available())
  {
    PreviousSerialAvailable = millis();
    incomingByte = SECSerial->read();
    if (incomingByte == STX)
    {
      InsideMessageFlag = true;
      ReceivedSTXFlag = true;
    }
    else if (incomingByte == ETX && InsideMessageFlag && ReceivedSTXFlag)
    {
      InsideMessageFlag = false;
      ReceivedSTXFlag = false;
      ProcessMessage(ReceiveMessageBuffer);

      ReceiveMessageBuffer = EMPTY_STRING;
    }
    else if (InsideMessageFlag)
    {
      ReceiveMessageBuffer += incomingByte;
    }
  }
  if (millis() - PreviousMillisProcessRegister > PROCESS_REQUESTREGISTER_DELAY_MILLIS)
  {
    ProcessRequestNextRegister();
  }

  if (millis() - PreviousMillisAckReceived > RESET_ACK_MILLIS)
  {
    PreviousMillisAckReceived = millis();
    LastMessageAccepted = true;
  }

  ProcessSendMessageAck();
  if (LastMessageAccepted)
  {
    if (millis() - PreviousSerialAvailable > PROCESS_SENDBUFFER_DELAY_MILLIS)
    {
      ProcessMessageSendBuffer();
    }
  }
}
