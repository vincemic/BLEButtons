#pragma once
#include <BluetoothSerial.h>
#include <ArduinoJson.h>
#include "LoopbackStream.h"

#define BUFFER_SIZE 500

class ProtocolProcessor
{

private:
    
    uint8_t markerCount = 0;
    LoopbackStream protocolStream;
    DynamicJsonDocument jsonDocument;
    BluetoothSerial *serialBT;
    typedef void (*ProtocolLedCallback)(uint8_t, bool);
    ProtocolLedCallback ledCallback;
    

    void process(uint8_t character);
    void processDocument();
    void logJsonDocument();

    void ExecuteLedCommand(JsonVariant commandJson);

public:
    ProtocolProcessor(ProtocolLedCallback ledCallback);
    void begin(BluetoothSerial *serialBT);
    void tick();
    
};