#pragma once
#include <ArduinoJson.h>

class SettingsClass
{

private:
    void save(DynamicJsonDocument &jsonDocument);
    bool load(DynamicJsonDocument &jsonDocument);

public:
    SettingsClass();
    void being();

    void writeWiFiSID(const char *name);
    void writeWiFiPassword(const char *password);
    void readWiFiSID(String &wifiSID);
    void readWiFiPassword(String &password);
    void clear();
    void report(JsonDocument &jsonDocument);
};

extern SettingsClass Settings;
