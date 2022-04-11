#include <ArduinoLog.h>
#include "SettingHandler.h"
#include "FileSystem.h"
#include "FileSystem.h"
#include "LoopbackStream.h"


#define WIFISID "wifisid"
#define WIFIPASSWORD "wifipassword"
#define SETTINGSPATH "/settings.json"
#define MAXSETTINGSIZE 1000

SettingHandlerClass::SettingHandlerClass()
{
}

void SettingHandlerClass::being()
{
    Log.traceln(F("[Settings] Starting file system on SPIFFS"));
    FileSystem.begin();
}

void SettingHandlerClass::clear()
{
    FileSystem.deleteFile(SETTINGSPATH);
}

void SettingHandlerClass::save(SpiRamJsonDocument &jsonDocument)
{
    String json;
    serializeJson(jsonDocument, json);
    Log.traceln(F("[Settings] Saving settings file\r%s"), json.c_str());
    FileSystem.writeFile(SETTINGSPATH, json.c_str());
}

bool SettingHandlerClass::load(SpiRamJsonDocument &jsonDocument)
{
    char buffer[MAXSETTINGSIZE];
    size_t fileSize = FileSystem.readFile(SETTINGSPATH, buffer, MAXSETTINGSIZE - 1);

    if (fileSize <= 0)
    {
        Log.errorln(F("[Settings] Cannot open settings file: %s"), SETTINGSPATH);
        return false;
    }

    DeserializationError error = deserializeJson(jsonDocument, buffer, fileSize);

    if (error)
    {
        Log.errorln(F("[Settings] Settings deserializeJson() failed: %s"), error.f_str());
        return false;
    }

    String json;
    serializeJson(jsonDocument, json);
    Log.traceln(F("[Settings] Settings file loaded: %s"), json.c_str());
    return true;
}

void SettingHandlerClass::writeWiFiSID(const char *name)
{
    SpiRamJsonDocument jsonDocument(MAXSETTINGSIZE);
    load(jsonDocument);
    jsonDocument[WIFISID] = name;
    save(jsonDocument);
}

void SettingHandlerClass::writeWiFiPassword(const char *password)
{
    SpiRamJsonDocument jsonDocument(MAXSETTINGSIZE);
    load(jsonDocument);
    jsonDocument[WIFIPASSWORD] = password;
    save(jsonDocument);
}

void SettingHandlerClass::readWiFiSID(String &wifiSID)
{
    SpiRamJsonDocument jsonDocument(MAXSETTINGSIZE);
    load(jsonDocument);
    wifiSID.concat(jsonDocument[WIFISID].as<const char *>());
}

void SettingHandlerClass::readWiFiPassword(String &password)
{
    SpiRamJsonDocument jsonDocument(MAXSETTINGSIZE);
    load(jsonDocument);
    password.concat(jsonDocument[WIFIPASSWORD].as<const char *>());
}

void SettingHandlerClass::report(SpiRamJsonDocument &jsonDocument)
{
    SpiRamJsonDocument settingsJsonDocument(MAXSETTINGSIZE);
    load(settingsJsonDocument);

    jsonMerge(jsonDocument, settingsJsonDocument,"settings");
}

SettingHandlerClass SettingHandler;
