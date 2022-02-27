#pragma once
#include <BluetoothSerial.h>
#include <ArduinoJson.h>
#include "LoopbackStream.h"

#define BUFFER_SIZE 500
typedef void (*ProtocolLedCallback)(uint8_t, bool);
class ProtocolProcessor
{

private:
   // typedef void (ProtocolProcessor::*CommandFunction)(void);
    typedef std::function<void()> CommandFunction;
    struct Command
    {
        CommandFunction commandFunction;
        const char *name;

        Command(const char *name,CommandFunction commandFunction)
        {
            this->name = name;
            this->commandFunction = commandFunction;
        }
    };

    uint8_t markerCount = 0;
    LoopbackStream protocolStream;
    DynamicJsonDocument jsonDocument;
    BluetoothSerial *serialBT;
    ProtocolLedCallback ledCallback;
    Command **commands;

    void process(uint8_t character);
    void processDocument();
    void logJsonDocument();

public:
    ProtocolProcessor(ProtocolLedCallback ledCallback);
    void begin(BluetoothSerial *serialBT);
    void tick();
    void sendButtonPress(uint8_t buttonNumber);

    void ExecuteLedCommand();
    void ExecuteSetWiFiSIDCommand();
    void ExecuteSetWiFiPasswordCommand();
    void ExecuteClearSettingsCommand();
};