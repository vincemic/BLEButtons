#pragma once
#include <RTClib.h>

class RTCHandlerClass
{
public:
    void begin();

private:
    RTC_PCF8523 rtc;
};

extern RTCHandlerClass RTCHandler;