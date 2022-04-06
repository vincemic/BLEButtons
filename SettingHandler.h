#pragma once
#include <ArduinoJson.h>

class SettingHandlerClass
{

private:
    void save(DynamicJsonDocument &jsonDocument);
    bool load(DynamicJsonDocument &jsonDocument);

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
