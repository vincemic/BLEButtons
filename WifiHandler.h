#pragma once
#include <ArduinoJson.h>
class WifiHandlerClass
{

public:
    WifiHandlerClass();
    bool connect();
    void disconnect();
    void tick();
    void report(JsonDocument &jsonDocument);
};

extern WifiHandlerClass WifiHandler;