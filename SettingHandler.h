#pragma once
#include <ArduinoJson.h>
#include "JsonHelper.h"

#define WIFISID "wifisid"
#define WIFIPASSWORD "wifipassword"
#define MAXSETTINGSIZE 1000
class SettingHandlerClass
{

private:
    SpiRamJsonDocument *pJsonDocument;
    char *buffer = NULL;
    bool mustSave = false;


public:
    SettingHandlerClass();
    void being();
    void save();
    bool load();
    void read(const char *name, std::string &value);
    void write(const char *name, const char *value);
    void readWiFiSID(std::string &wifiSID);
    void readWiFiPassword(std::string &password);
    void clear();
    void report(SpiRamJsonDocument &jsonDocument);
};

extern SettingHandlerClass SettingHandler;
