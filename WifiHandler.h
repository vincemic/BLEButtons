#pragma once
#include <ArduinoJson.h>
#include "JsonHelper.h"
class WifiHandlerClass
{

public:
    WifiHandlerClass();
    void connect();
    void disconnect();
    void tick();
    void report(SpiRamJsonDocument &jsonDocument);
    void getFile(const char *filepath);
    void createTTSFile(const char *text, const char *label);
    bool isConnected();

private:
    void setClock();
    bool didInit = false;
};

extern WifiHandlerClass WifiHandler;