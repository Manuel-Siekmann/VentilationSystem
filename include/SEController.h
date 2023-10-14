/*
  This file is part of the VentilationSystem project.
  Copyright (C) 2023 Dr. Manuel Siekmann. All rights reserved.
*/

#ifndef SECONTROLLER_H
#define SECONTROLLER_H

#include <SoftwareSerial.h>

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

#define AREA_LEVEL_START 173
#define AREA_LEVEL_END 178

#define REGISTER_MAX 256

#define ON_REGISTERCHANGED_MAX 10

#define EMPTY_STRING ""

class SEController
{
private:
  bool InsideMessageFlag = false;
  bool ReceivedSTXFlag = false;

  bool LastMessageAccepted = true;
  bool SendMessageAck = false;

  unsigned int RegisterIndex = AREA_LEVEL_START;

  unsigned long PreviousMillisProcessRegister = 0;
  unsigned long PreviousMillisAckReceived = 0;
  unsigned long PreviousSerialAvailable = 0;

  typedef std::function<void(SEController*, int, String)> RegisterChangedCallback;
  RegisterChangedCallback OnRegisterChanged[ON_REGISTERCHANGED_MAX];
  unsigned int OnRegisterChangedCount = 0;
  
  String Register[REGISTER_MAX];
  String SendMessageBuffer = EMPTY_STRING;
  String ReceiveMessageBuffer = EMPTY_STRING;

  SoftwareSerial *SECSerial;

  bool IsSendBufferEmpty();
  void SendMessageRequest(int commandId, int registerId);
  void ProcessMessageResponseIncome(int commandId, int registerId, String content);
  void ProcessSendMessageAck();
  void ProcessMessage(String message);
  void ProcessRequestNextRegister();
  void ProcessMessageSendBuffer();

public:
  SEController(uint8_t rxPin, uint8_t txPin);
  ~SEController();
  void SendMessageResponse(int registerId, String content);
  void AddOnRegisterChanged(RegisterChangedCallback callback);
  void Poll();
};

#endif
