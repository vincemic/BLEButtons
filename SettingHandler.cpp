#include <ArduinoLog.h>
#include "SettingHandler.h"
#include "FileSystem.h"
#include "FileSystem.h"

#define SETTINGSPATH "/settings.json"

SettingHandlerClass::SettingHandlerClass()
{
}

void SettingHandlerClass::being()
{
    pJsonDocument = new SpiRamJsonDocument(100);
    buffer = (char *)heap_caps_malloc(MAXSETTINGSIZE, MALLOC_CAP_SPIRAM);

    Log.traceln(F("[Settings] Starting file system on SPIFFS"));
    FileSystem.begin();
    load();

    xTaskCreatePinnedToCore(
        [](void *parameter)
        {
            while (true)
            {
                SettingHandler.save();
                vTaskDelay(40);
            }
        },
        "settingsSaveTask", // A name just for humans
        4096,               // This stack size can be checked & adjusted by reading the Stack Highwater
        NULL,               //
        2,                  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
        &taskHandle,
        0);
}

void SettingHandlerClass::clear()
{
    FileSystem.deleteFile(SETTINGSPATH);
}

void SettingHandlerClass::save()
{
    if (mustSave)
    {
        mustSave = false;
        String json;
        serializeJson(*pJsonDocument, json);
        Log.traceln(F("[Settings] Saving settings file\r%s"), json.c_str());
        FileSystem.writeFile(SETTINGSPATH, json.c_str());
    }
}

bool SettingHandlerClass::load()
{
    mustSave = false;

    size_t fileSize = FileSystem.readFile(SETTINGSPATH, buffer, MAXSETTINGSIZE - 1);
    Log.errorln(F("[Settings] Json document capacity: %d, file size: %d"), pJsonDocument->capacity(), fileSize);

    if (fileSize <= 0)
    {
        Log.errorln(F("[Settings] Cannot open settings file: %s"), SETTINGSPATH);
        return false;
    }

    DeserializationError error = deserializeJson(*pJsonDocument, buffer, fileSize);

    if (error)
    {
        Log.errorln(F("[Settings] Settings deserializeJson() failed: %s"), error.f_str());
        return false;
    }

    String json;
    serializeJson(*pJsonDocument, json);
    Log.traceln(F("[Settings] Reading key from: %s"), json.c_str());

    return true;
}

void SettingHandlerClass::read(const char *name, std::string &value)
{
    const char *docValue = (*pJsonDocument)[name];

    Log.traceln(F("[SettingHandler] Reading key: %s, value: %s"), name, docValue);

    if (docValue != NULL)
    {
        value.append(docValue);
    }
    else
    {
        value.append("");
    }
}

void SettingHandlerClass::write(const char *name, const char *value)
{
    String newValue(value);
    Log.traceln(F("[SettingHandler] setting key: %s, value: %s"), name, value);
    (*pJsonDocument)[name] = newValue;
    mustSave = true;
}

void SettingHandlerClass::readWiFiSID(std::string &wifiSID)
{
    read(WIFISID, wifiSID);
}

void SettingHandlerClass::readWiFiPassword(std::string &password)
{
    read(WIFIPASSWORD, password);
}

void SettingHandlerClass::report(SpiRamJsonDocument &jsonDocument)
{
    jsonMerge(jsonDocument, *pJsonDocument, "settings");
}

SettingHandlerClass SettingHandler;
