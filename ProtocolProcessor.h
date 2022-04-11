#pragma once
#include <ArduinoJson.h>
#include "LoopbackStream.h"
#include "JsonHelper.h"

#define BUFFER_SIZE 500
typedef void (*ProtocolLedCallback)(uint8_t, bool);
typedef void (*ProtocolReportCallbback)();
typedef void (*ProtocolPlayCallbback)(const char *filename);
class ProtocolProcessor
{

private:
    typedef std::function<void(SpiRamJsonDocument &jsonDocument)> CommandFunction;
    struct Command
    {
        CommandFunction commandFunction;
        const char *name;

        Command(const char *name, CommandFunction commandFunction)
        {
            this->name = name;
            this->commandFunction = commandFunction;
        }
    };

    uint8_t markerCount = 0;
    ProtocolLedCallback ledCallback;
    ProtocolReportCallbback reportCallback;
    ProtocolPlayCallbback playCallback;
    Command **commands;
    SpiRamJsonDocument  *jsonDocument;

public:
    ProtocolProcessor(ProtocolLedCallback ledCallback, ProtocolReportCallbback reportCallback, ProtocolPlayCallbback playCallback);

    void sendStatus(const char *type, const char *status, uint8_t number, bool isAck = false);
    void sendStatus(const char *type, const char *status, const char *detail, bool isAck = false);
    void send(JsonDocument &jsonDocument);
    void process(const char *message,size_t size);
    void begin();

    void ExecuteLedCommand(SpiRamJsonDocument &jsonDocument);
    void ExecuteSetWiFiSIDCommand(SpiRamJsonDocument &jsonDocument);
    void ExecuteSetWiFiPasswordCommand(SpiRamJsonDocument &jsonDocument);
    void ExecuteClearSettingsCommand(SpiRamJsonDocument &jsonDocument);
    void ExecuteReportCommand(SpiRamJsonDocument &jsonDocument);
    void ExecutePlayCommand(SpiRamJsonDocument &jsonDocument);
};