#pragma once
#include <BluetoothSerial.h>
#include <ArduinoJson.h>
#include "LoopbackStream.h"

#define BUFFER_SIZE 500
typedef void (*ProtocolLedCallback)(uint8_t, bool);
typedef void (*ProtocolReportCallbback)();
class ProtocolProcessor
{

private:
   
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
    ProtocolReportCallbback reportCallback;
    Command **commands;

    void process(uint8_t character);
    void processDocument();
    void logJsonDocument();

public:
    ProtocolProcessor(ProtocolLedCallback ledCallback, ProtocolReportCallbback reportCallback);
    void begin(BluetoothSerial *serialBT);
    void tick();

    void sendStatus(const char * type, const char *status, uint8_t number , bool isAck = false);
    void send(JsonDocument &jsonDocument);

    void ExecuteLedCommand();
    void ExecuteSetWiFiSIDCommand();
    void ExecuteSetWiFiPasswordCommand();
    void ExecuteClearSettingsCommand();
    void ExecuteReportCommand();

};