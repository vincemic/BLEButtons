#include "WifiHandler.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <SD.h>
#include "SettingHandler.h"
#include "ArduinoLog.h"

WifiHandlerClass::WifiHandlerClass()
{
}

void WifiHandlerClass::connect()
{
    std::string sid;
    std::string password;
    wl_status_t status = WL_NO_SHIELD;

    SettingHandler.readWiFiSID(sid);
    SettingHandler.readWiFiPassword(password);

    disconnect();

    Log.noticeln(F("Connecting to %s ...."), sid.c_str());

    status = WiFi.begin(sid.c_str(), password.c_str());

}

void WifiHandlerClass::disconnect()
{

    if (WiFi.status() == WL_CONNECTED)
    {
        WiFi.disconnect();

        Log.noticeln(F("WiFi disconnected"));
    }
}

void WifiHandlerClass::tick()
{
    wl_status_t status = WiFi.status();

    switch (status)
    {
    case WL_CONNECTED:
        Log.noticeln(F("Connected to WiFI (%s:%s)"), WiFi.getHostname(), WiFi.localIP().toString());
        break;
    case WL_CONNECT_FAILED:
        Log.noticeln(F("WiFi connection failed"));
        break;
    default:
        Log.noticeln(F("Could not connect to WiFi status code: %d"), status);
    }
}

void WifiHandlerClass::report(SpiRamJsonDocument &jsonDocument)
{

    jsonDocument["wifi"]["status"] = WiFi.status();

    if (WiFi.isConnected())
    {
        jsonDocument["wifi"]["rssi"] = WiFi.RSSI();
        jsonDocument["wifi"]["hostName"] = WiFi.getHostname();
        jsonDocument["wifi"]["ip"] = WiFi.localIP().toString();
        jsonDocument["wifi"]["mac"] = WiFi.macAddress();
    }
}

void WifiHandlerClass::getFile(const char *filepath)
{
    WiFiClientSecure wifiClient;
    wifiClient.setInsecure();
    HTTPClient https;
    String path = String(F("https://bitdogttsbucket.s3.amazonaws.com"));

    path.concat(filepath);

    Log.traceln(F("[HTTPS] begin..."));
    if (https.begin(wifiClient,path))
    {
        // HTTPS
        Log.traceln(F("[HTTPS] GET..."));
        // start connection and send HTTP header
        int httpCode = https.GET();

        // httpCode will be negative on error
        if (httpCode > 0)
        {
            // HTTP header has been send and Server response header has been handled
            Log.traceln(F("[HTTPS] GET... code: %d"), httpCode);

            // file found at server
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
            {
                Log.traceln(F("[HTTPS] Saving to file: %s"), filepath);
                File file = SD.open(filepath, "w");

                if (file)
                    https.writeToStream(&file);
                else
                    Log.traceln(F("[HTTPS] Could not open SD file: %s"), filepath);
            }
        }
        else
        {
            Log.errorln(F("[HTTPS] GET... failed, error: %s"), https.errorToString(httpCode).c_str());
        }

        https.end();
    }
    else
    {
        Log.errorln(F("[HTTPS] Unable to create client"));
    }
}
WifiHandlerClass WifiHandler;
