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

bool WifiHandlerClass::connect()
{
    unsigned long startMillis = millis();
    std::string sid;
    std::string password;
    wl_status_t status = WL_NO_SHIELD;

    SettingHandler.readWiFiSID(sid);
    SettingHandler.readWiFiPassword(password);

    Log.notice(F("[WifiHandler] Connecting to %s ...."), sid.c_str());

    WiFi.mode(WIFI_STA);
    WiFi.setHostname(HOSTNAME);
    //   WiFi.enableLongRange(true);
    //   WiFi.setSleep(false);

    Log.notice(F("."));

    WiFi.begin(sid.c_str(), password.c_str());
    Log.noticeln(F("."));

    while (WiFi.status() != WL_CONNECTED)
    {
        Log.notice(F("."));
        delay(1000);

        if (millis() - startMillis > 30000)
        {
            Log.noticeln(F("\n[WifiHandler] Failed to connect to WiFI"));
            return false;
        }
    }

    Log.noticeln(F("\n[WifiHandler] Connected to WiFI (%s:%s)"), WiFi.getHostname(), WiFi.localIP().toString());

    return true;
}

void WifiHandlerClass::disconnect()
{

    if (WiFi.status() == WL_CONNECTED)
    {
        WiFi.disconnect(true, true);

        Log.noticeln(F("[WifiHandler] WiFi disconnected"));
    }
}

void WifiHandlerClass::tick()
{
    wl_status_t status = WiFi.status();

    switch (status)
    {
    case WL_CONNECTED:
        Log.noticeln(F("[WifiHandler] Connected to WiFI (%s:%s)"), WiFi.getHostname(), WiFi.localIP().toString());

        if (!didInit)
        {
            didInit = true;
            setClock();
        }
        break;
    case WL_CONNECT_FAILED:
        Log.noticeln(F("[WifiHandler] WiFi connection failed"));
        break;
    default:
        Log.noticeln(F("[WifiHandler] Could not connect to WiFi status code: %d"), status);
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

void WifiHandlerClass::createTTSFile(const char *text, const char *label)
{
    SpiRamJsonDocument jsonDocument(200);
    WiFiClientSecure wifiClient;
    wifiClient.setInsecure();
    HTTPClient https;
    String path = String(F("https://cq3odfu0dg.execute-api.us-east-1.amazonaws.com/Production/"));
    String json;

    jsonDocument["text"] = text;
    jsonDocument["label"] = label;
    serializeJson(jsonDocument, json);

    Log.traceln(F("[HTTPS] begin..."));
    if (https.begin(wifiClient, path))
    {
        https.setTimeout(45000);
        // HTTPS
        Log.traceln(F("[HTTPS] POST..."));
        // start connection and send HTTP header
        int httpCode = https.POST(json);

        // httpCode will be negative on error
        if (httpCode == HTTP_CODE_OK)
        {
            // HTTP header has been send and Server response header has been handled
            Log.traceln(F("[HTTPS] POST... code: %d"), httpCode);
        }
        else
        {
            Log.errorln(F("[HTTPS] POST... failed, code: %s"), https.errorToString(httpCode).c_str());
        }
    }
    else
    {
        Log.errorln(F("[HTTPS] Unable to create client"));
    }

    https.end();
}

void WifiHandlerClass::getFile(const char *filepath)
{
    WiFiClientSecure wifiClient;
    wifiClient.setInsecure();
    HTTPClient https;
    String path = String(F("https://bitdogttsbucket.s3.amazonaws.com"));

    path.concat(filepath);

    Log.traceln(F("[HTTPS] begin..."));
    if (https.begin(wifiClient, path))
    {
        https.setTimeout(45000);
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

        Log.errorln(F("[HTTPS] Saved file: %s"), filepath);
    }
    else
    {
        Log.errorln(F("[HTTPS] Unable to create client"));
    }

    https.end();
}

void WifiHandlerClass::setClock()
{
    struct tm timeinfo;
    configTime(-4 * 60 * 60, 0, "pool.ntp.org");
    Log.infoln(F("Waiting for NTP time sync: "));
    getLocalTime(&timeinfo);
    Log.infoln(F("Current time: %s"), asctime(&timeinfo));
}

bool WifiHandlerClass::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

WifiHandlerClass WifiHandler;
