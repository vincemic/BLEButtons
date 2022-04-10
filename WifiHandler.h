#pragma once
#include <ArduinoJson.h>
class WifiHandlerClass
{

public:
    WifiHandlerClass();
    void connect();
    void disconnect();
    void tick();
    void report(JsonDocument &jsonDocument);
    void getFile(const char *filepath);
};

extern WifiHandlerClass WifiHandler;