#include "ProtocolProcessor.h"
#include <ArduinoLog.h>

ProtocolProcessor::ProtocolProcessor(ProtocolLedCallback ledCallback) 
: jsonDocument(50),
protocolStream(50)
{
    this->ledCallback = ledCallback;
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
        return;
    }
    else
    {

        logJsonDocument();
        Log.traceln(F("message received"));

        JsonVariant commandJson = jsonDocument["command"];

        if (commandJson.isNull())
        {
            Log.traceln(F("command not found"));
            return;
        }
        else
        {
            const char *commandName = commandJson.as<const char *>();
            Log.traceln(F("command found: %s"), commandName);

            if(strcmp(commandName,"led")==0) {
               ExecuteLedCommand(commandJson);
            }
        }
    }
}

void ProtocolProcessor::logJsonDocument()
{
    serializeJson(jsonDocument, Serial);
    Log.trace(CR);
}

void ProtocolProcessor::ExecuteLedCommand(JsonVariant commandJson) {
    if(ledCallback != NULL)
        ledCallback(1,true);
}