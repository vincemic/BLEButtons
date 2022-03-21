#include "WifiHandler.h"
#include <WiFi.h>
#include "settings.h"
#include "ArduinoLog.h"

WifiHandlerClass::WifiHandlerClass()
{
}

bool WifiHandlerClass::connect()
{
    String sid;
    String password;
    wl_status_t status = WL_NO_SHIELD;

    Settings.readWiFiSID(sid);
    Settings.readWiFiPassword(password);

    disconnect();

    Log.noticeln(F("Connecting to %s ...."), sid);

    status = WiFi.begin(sid.c_str(), password.c_str());

    while(status != WL_CONNECTED && status != WL_CONNECT_FAILED) {
        delay(500);
        status = WiFi.status();
    }

    return status == WL_CONNECTED;
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

void WifiHandlerClass::report(JsonDocument &jsonDocument)
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

WifiHandlerClass WifiHandler;
