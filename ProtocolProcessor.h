#pragma once
#include <ArduinoJson.h>
#include "LoopbackStream.h"

#define BUFFER_SIZE 500
typedef void (*ProtocolLedCallback)(uint8_t, bool);
typedef void (*ProtocolReportCallbback)();
typedef void (*ProtocolPlayCallbback)(const char *filename);
class ProtocolProcessor
{

private:
    typedef std::function<void()> CommandFunction;
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
    LoopbackStream protocolStream;
    DynamicJsonDocument jsonDocument;
    ProtocolLedCallback ledCallback;
    ProtocolReportCallbback reportCallback;
    ProtocolPlayCallbback playCallback;
    Command **commands;

    void process(uint8_t character);
    void processDocument();
    void logJsonDocument();

public:
    ProtocolProcessor(ProtocolLedCallback ledCallback, ProtocolReportCallbback reportCallback, ProtocolPlayCallbback playCallback);
    void begin();
    void tick();

    void sendStatus(const char *type, const char *status, uint8_t number, bool isAck = false);
    void sendStatus(const char *type, const char *status, const char *detail, bool isAck = false);
    void send(JsonDocument &jsonDocument);

    void ExecuteLedCommand();
    void ExecuteSetWiFiSIDCommand();
    void ExecuteSetWiFiPasswordCommand();
    void ExecuteClearSettingsCommand();
    void ExecuteReportCommand();
    void ExecutePlayCommand();
};