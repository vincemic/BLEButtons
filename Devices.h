#pragma once

#include <Adafruit_seesaw.h>

#define DEFAULT_I2C_ADDR 0x3A
typedef void (*ButtonChangeCallback)(uint8_t, bool);

class Devices
{

private:
    Adafruit_seesaw seesaw;
    uint16_t pid;
    uint8_t year, mon, day;
    bool seesawReady = false;
    ButtonChangeCallback buttonChangeCallback;


    bool startSeesaw();

public:
    Devices(ButtonChangeCallback buttonChangeCallback);
    void tick();
    void begin();
    void turnOnLED(uint8_t number);
    void turnOffLED(uint8_t number);

};