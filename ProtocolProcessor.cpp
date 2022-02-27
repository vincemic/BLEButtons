#include "ProtocolProcessor.h"
#include <ArduinoLog.h>

ProtocolProcessor::ProtocolProcessor(ProtocolLedCallback ledCallback) 
: jsonDocument(500),
protocolStream(500)
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

        const char* commandName = jsonDocument["command"];

        if (commandName == NULL)
        {
            Log.traceln(F("command not found"));
            return;
        }
        else
        {
            Log.traceln(F("command found: %s"), commandName);

            if(strcmp(commandName,"led")==0) {
               ExecuteLedCommand();
            }
        }
    }
}

void ProtocolProcessor::logJsonDocument()
{
    serializeJson(jsonDocument, Serial);
    Log.trace(CR);
}

void ProtocolProcessor::ExecuteLedCommand() {

    uint8_t turnOn = jsonDocument["turnOn"];
    uint8_t turnOff = jsonDocument["turnOff"];
    uint8_t number = 0;
    bool on = false;

    if(turnOn == 0 && turnOff == 0 ){
        Log.errorln(F("Incorrect protocol"));
        return;
    }

    if(turnOn != 0) {
         number = turnOn;
         on = true;
    }
    else {
         number = turnOff;
         on = false;
    }
    
    if(ledCallback != NULL)
        ledCallback(number,on);
}

void ProtocolProcessor::sendButtonPress(uint8_t buttonNumber) {
    char buffer[32];
    snprintf(buffer, 32, "{ buttonPress: %d}\r\r", buttonNumber);

    for(uint8_t i = 0; i < 32; i++) {

        if(buffer[i] == 0)
            break;

        serialBT->write(buffer[i]);
    }
}
