#include "ProtocolProcessor.h"
#include <ArduinoLog.h>
#include "SettingHandler.h"
#include "BLEHandler.h"

#define COMMAND_COUNT 6

ProtocolProcessor::ProtocolProcessor(ProtocolLedCallback ledCallback, ProtocolReportCallbback reportCallback, ProtocolPlayCallbback playCallback)
{

    this->ledCallback = ledCallback;
    this->reportCallback = reportCallback;
    this->playCallback = playCallback;

    this->commands = new Command *[COMMAND_COUNT]
    {
        new Command("led", [this](SpiRamJsonDocument &jsonDocument)
                    { ExecuteLedCommand(jsonDocument); }),
            new Command("wifisid", [this](SpiRamJsonDocument &jsonDocument)
                        { ExecuteSetWiFiSIDCommand(jsonDocument); }),
            new Command("wifipassword", [this](SpiRamJsonDocument &jsonDocument)
                        { ExecuteSetWiFiPasswordCommand(jsonDocument); }),
            new Command("clearsettings", [this](SpiRamJsonDocument &jsonDocument)
                        { ExecuteClearSettingsCommand(jsonDocument); }),
            new Command("report", [this](SpiRamJsonDocument &jsonDocument)
                        { ExecuteReportCommand(jsonDocument); }),
            new Command("play", [this](SpiRamJsonDocument &jsonDocument)
                        { ExecutePlayCommand(jsonDocument); })
    };
}

void ProtocolProcessor::begin() {
    jsonDocument = new SpiRamJsonDocument(250);
}

void ProtocolProcessor::process(const char *message,size_t size)
{

    Log.errorln(F("[Protocol Processor] got meesage size: %d"),size);

    DeserializationError error = deserializeJson(*jsonDocument, message, size);

    if (error)
    {
        Log.errorln(F("[Protocol Processor] deserializeJson() failed: %s"), error.f_str());
    }
    else
    {

        Log.traceln(F("[Protocol Processor] Message received"));

        const char *commandName = (*jsonDocument)["command"];

        if (commandName == NULL)
        {
            Log.traceln(F("[Protocol Processor] Command not found"));
        }
        else
        {
            Log.traceln(F("[Protocol Processor] Command found: %s"), commandName);

            for (int index = 0; index < COMMAND_COUNT; index++)
            {
                if (strcmp(commands[index]->name, commandName) == 0)
                {
                    commands[index]->commandFunction(*jsonDocument);
                    break;
                }
            }
        }
    }
}


void ProtocolProcessor::sendStatus(const char *type, const char *status, uint8_t number, bool isAck)
{
    SpiRamJsonDocument jsonDocument(100);
    jsonDocument["type"] = type;
    jsonDocument["status"] = status;
    jsonDocument["number"] = number;
    jsonDocument["isAck"] = isAck;
    send(jsonDocument);
}

void ProtocolProcessor::sendStatus(const char *type, const char *status, const char *detail, bool isAck)
{
    SpiRamJsonDocument jsonDocument(100);
    jsonDocument["type"] = type;
    jsonDocument["status"] = status;
    jsonDocument["detail"] = detail;
    jsonDocument["isAck"] = isAck;
    send(jsonDocument);
}

void ProtocolProcessor::send(JsonDocument &jsonDocument)
{
    String json;
    serializeJson(jsonDocument, json);
    json.concat("\r\r");
    BLEHandler.send(json.c_str());
    Log.traceln("%s", json.c_str());
}

//////////////////////////////

void ProtocolProcessor::ExecuteLedCommand(SpiRamJsonDocument &jsonDocument)
{

    uint8_t on = jsonDocument["on"];
    uint8_t off = jsonDocument["off"];
    uint8_t number = 0;
    bool result = false;

    if (on == 0 && off == 0)
    {
        Log.errorln(F("Incorrect protocol"));
        return;
    }

    if (on != 0)
    {
        number = on;
        result = true;
    }
    else
    {
        number = off;
        result = false;
    }

    if (ledCallback != NULL)
        ledCallback(number, result);
}

void ProtocolProcessor::ExecuteSetWiFiSIDCommand(SpiRamJsonDocument &jsonDocument)
{
    const char *sidName = jsonDocument["sid"];
    SettingHandler.writeWiFiSID(sidName);
}

void ProtocolProcessor::ExecuteSetWiFiPasswordCommand(SpiRamJsonDocument &jsonDocument)
{

    const char *password = jsonDocument["password"];
    SettingHandler.writeWiFiPassword(password);
}

void ProtocolProcessor::ExecuteClearSettingsCommand(SpiRamJsonDocument &jsonDocument)
{
    SettingHandler.clear();
}

void ProtocolProcessor::ExecuteReportCommand(SpiRamJsonDocument &jsonDocument)
{
    if (reportCallback != NULL)
        reportCallback();
}

void ProtocolProcessor::ExecutePlayCommand(SpiRamJsonDocument &jsonDocument)
{
    const char *filename = jsonDocument["filename"];
    if (playCallback != NULL)
        playCallback(filename);
}