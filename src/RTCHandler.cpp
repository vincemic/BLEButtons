#include "RTCHandler.h"
#include <ArduinoLog.h>

void RTCHandlerClass::begin()
{
    if (!rtc.begin())
    {
        Log.errorln(F("[RTCHandler] Couldn't find RTC"));
    } else {
        Log.errorln(F("[RTCHandler] Found RTC"));
    }
}

RTCHandlerClass RTCHandler;