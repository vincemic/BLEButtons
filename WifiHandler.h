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
};

extern WifiHandlerClass WifiHandler;