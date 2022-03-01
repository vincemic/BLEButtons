#include <ArduinoLog.h>
#include "Settings.h"
#include "FileSystem.h"
#include "ArduinoJson.h"
#include "FileSystem.h"
#include "LoopbackStream.h"

#define WIFISID "WIFISID"
#define WIFIPASSWORD "WIFIPASSWORD"
#define SETTINGSPATH "/settings.json"
#define MAXSETTINGSIZE 1000

SettingsClass::SettingsClass()
{
}

void SettingsClass::being()
{
    FileSystem.begin();
}

void SettingsClass::clear()
{
    FileSystem.deleteFile(SETTINGSPATH);
}

void SettingsClass::save(JsonDocument *jsonDocument)
{
    String json;
    serializeJson(*jsonDocument, json);
    Log.traceln(F("Saving settings file\r%s"), json.c_str());
    FileSystem.writeFile(SETTINGSPATH, json.c_str());
}

bool SettingsClass::load(JsonDocument *jsonDocument)
{
    char buffer[MAXSETTINGSIZE];
    memset(buffer,'\0',MAXSETTINGSIZE);

    size_t fileSize = FileSystem.readFile(SETTINGSPATH, buffer, MAXSETTINGSIZE -1);

    jsonDocument->clear();

    if (fileSize <= 0)
    {
        Log.errorln(F("Cannot open settings file: %s"), SETTINGSPATH);
        return false;
    }


    DeserializationError error = deserializeJson(*jsonDocument, buffer, fileSize);

    if (error)
    {
        Log.errorln(F("Settings deserializeJson() failed: %s"), error.f_str());
        jsonDocument->clear();
        return false;
    }

    String json;
    serializeJson(*jsonDocument, json);
    Log.traceln(F("Settings file loaded\r%s"), json.c_str());
    return true;
}

void SettingsClass::writeWiFiSID(const char *name)
{   
    DynamicJsonDocument jsonDocument(MAXSETTINGSIZE);
    load(&jsonDocument);
    jsonDocument[WIFISID] = name;
    save(&jsonDocument);
}

void SettingsClass::writeWiFiPassword(const char *password)
{
    DynamicJsonDocument jsonDocument(MAXSETTINGSIZE);
    load(&jsonDocument);
    jsonDocument[WIFIPASSWORD] = password;
    save(&jsonDocument);
}

void SettingsClass::readWiFiSID(String * wifiSID)
{
    DynamicJsonDocument jsonDocument(MAXSETTINGSIZE);
    load(&jsonDocument);
    wifiSID->concat(jsonDocument[WIFISID].as<const char *>());
}

void SettingsClass::readWiFiPassword(String *password)
{
    DynamicJsonDocument jsonDocument(MAXSETTINGSIZE);
    load(&jsonDocument);
    password->concat(jsonDocument[WIFIPASSWORD].as<const char *>());
}

SettingsClass Settings;