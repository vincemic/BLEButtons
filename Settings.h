#pragma once
#include <ArduinoJson.h>

class SettingsClass
{

private:
    void save(JsonDocument *jsonDocument);
    bool load(JsonDocument *jsonDocument);

public:
    SettingsClass();
    void being();

    void writeWiFiSID(const char *name);
    void writeWiFiPassword(const char *password);
    void readWiFiSID(String *wifiSID);
    void readWiFiPassword(String *password);
    void clear();
};

extern SettingsClass Settings;
