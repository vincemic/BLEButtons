#pragma once
#include <ArduinoJson.h>
#include "JsonHelper.h"

class SettingHandlerClass
{

private:
    void save(SpiRamJsonDocument &jsonDocument);
    bool load(SpiRamJsonDocument &jsonDocument);

public:
    SettingHandlerClass();
    void being();

    void writeWiFiSID(const char *name);
    void writeWiFiPassword(const char *password);
    void readWiFiSID(String &wifiSID);
    void readWiFiPassword(String &password);
    void clear();
    void report(JsonDocument &jsonDocument);
};

extern SettingHandlerClass SettingHandler;
