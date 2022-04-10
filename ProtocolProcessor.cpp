#include "ProtocolProcessor.h"
#include <ArduinoLog.h>
#include "SettingHandler.h"
#include "BLEHandler.h"

#define COMMAND_COUNT 6

ProtocolProcessor::ProtocolProcessor(ProtocolLedCallback ledCallback, ProtocolReportCallbback reportCallback, ProtocolPlayCallbback playCallback)
    : jsonDocument(250)
{

    this->ledCallback = ledCallback;
    this->reportCallback = reportCallback;
    this->playCallback = playCallback;

    this->commands = new Command *[COMMAND_COUNT]
    {
        new Command("led", [this]()
                    { ExecuteLedCommand(); }),
            new Command("wifisid", [this]()
                        { ExecuteSetWiFiSIDCommand(); }),
            new Command("wifipassword", [this]()
                        { ExecuteSetWiFiPasswordCommand(); }),
            new Command("clearsettings", [this]()
                        { ExecuteClearSettingsCommand(); }),
            new Command("report", [this]()
                        { ExecuteReportCommand(); }),
            new Command("play", [this]()
                        { ExecutePlayCommand(); })
    };
}

void ProtocolProcessor::process(const char *message,size_t size)
{

    DeserializationError error = deserializeJson(jsonDocument, message, size);

    if (error)
    {
        Log.errorln(F("[Protocol Processor] deserializeJson() failed: %s"), error.f_str());
    }
    else
    {

        logJsonDocument();
        Log.traceln(F("message received"));

        const char *commandName = jsonDocument["command"];

        if (commandName == NULL)
        {
            Log.traceln(F("command not found"));
        }
        else
        {
            Log.traceln(F("command found: %s"), commandName);

            for (int index = 0; index < COMMAND_COUNT; index++)
            {
                if (strcmp(commands[index]->name, commandName) == 0)
                {
                    commands[index]->commandFunction();
                    break;
                }
            }
        }
    }
}

void ProtocolProcessor::logJsonDocument()
{
    String json;
    serializeJson(jsonDocument, json);
    Log.traceln("%s", json.c_str());
}

/////////////////////////////

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

void ProtocolProcessor::ExecuteLedCommand()
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

void ProtocolProcessor::ExecuteSetWiFiSIDCommand()
{
    const char *sidName = jsonDocument["sid"];
    SettingHandler.writeWiFiSID(sidName);
}

void ProtocolProcessor::ExecuteSetWiFiPasswordCommand()
{

    const char *password = jsonDocument["password"];
    SettingHandler.writeWiFiPassword(password);
}

void ProtocolProcessor::ExecuteClearSettingsCommand()
{
    SettingHandler.clear();
}

void ProtocolProcessor::ExecuteReportCommand()
{
    if (reportCallback != NULL)
        reportCallback();
}

void ProtocolProcessor::ExecutePlayCommand()
{
    const char *filename = jsonDocument["filename"];
    if (playCallback != NULL)
        playCallback(filename);
}