#include "ProtocolProcessor.h"
#include <ArduinoLog.h>
#include "Settings.h"
#define COMMAND_COUNT 5

ProtocolProcessor::ProtocolProcessor(ProtocolLedCallback ledCallback, ProtocolReportCallbback reportCallback)
    : jsonDocument(250),
      protocolStream(250)
{

    this->ledCallback = ledCallback;
    this->reportCallback = reportCallback;
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
                  { ExecuteReportCommand(); })
    };
}
void ProtocolProcessor::begin(BluetoothSerial *serialBT)
{
    this->serialBT = serialBT;
}

void ProtocolProcessor::tick()
{
    while (serialBT->available())
    {
        process(serialBT->read());
    }
}

void ProtocolProcessor::process(uint8_t character)
{

    if (character < 0)
        return;

    protocolStream.write(character);

    if (character == '\r')
        markerCount++;
    else
        markerCount = 0;

    if (markerCount >= 2)
    {
        processDocument();
        markerCount = 0;
    }
}

void ProtocolProcessor::processDocument()
{
    DeserializationError error = deserializeJson(jsonDocument, protocolStream);
    protocolStream.clear();

    if (error)
    {
        Log.errorln(F("deserializeJson() failed: %s"), error.f_str());
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
    Log.traceln("%s",json.c_str());
}

 /////////////////////////////


void ProtocolProcessor::sendStatus(const char * type, const char *status, uint8_t number )
{
    DynamicJsonDocument jsonDocument(100);
    String json;

    jsonDocument["type"] = type;
    jsonDocument["status"] = status;
    jsonDocument["number"] = number;

    serializeJson(jsonDocument, json);

    size_t count = json.length();
    const char *buffer = json.c_str();

    for (size_t i = 0; i < count; i++)
    {
        serialBT->write(buffer[i]);
    }
}

void ProtocolProcessor::sendReport(JsonDocument &jsonReportDocument)
{
    String json;
    serializeJson(jsonReportDocument, json);
    json.concat("\r\r");

    const char *jsonBuffer = json.c_str();
    Log.traceln("Sending requested report: %s", json.c_str());
    uint16_t length = json.length();

    uint16_t index = 0;

    while (index < length)
    {
        serialBT->write(jsonBuffer[index]);
        index++;
    }
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
    Settings.writeWiFiSID(sidName);
}

void ProtocolProcessor::ExecuteSetWiFiPasswordCommand()
{

    const char *password = jsonDocument["password"];
    Settings.writeWiFiPassword(password);
}

void ProtocolProcessor::ExecuteClearSettingsCommand() {
    Settings.clear();
}

void ProtocolProcessor::ExecuteReportCommand() {
    if(reportCallback != NULL)
        reportCallback();
}
