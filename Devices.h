#pragma once

#include <Adafruit_seesaw.h>

#define DEFAULT_I2C_ADDR 0x3A

class Devices
{

private:
    Adafruit_seesaw seesaw;
    uint16_t pid;
    uint8_t year, mon, day;



    bool startSeesaw();

public:
    Devices();
    void tick();
    void begin();
};