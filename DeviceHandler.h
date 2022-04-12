#pragma once

#include <Adafruit_seesaw.h>

#define DEFAULT_I2C_ADDR 0x3A
typedef void (*ButtonChangeCallback)(uint8_t, bool);

class Device
{
public:
    const __FlashStringHelper *bleIdentifier;
    const __FlashStringHelper *name;
    uint8_t index = 0;
};

class Button : public Device
{
public:
    uint8_t pin = 0;
    bool debounce = true;

    Button(uint8_t index, const __FlashStringHelper *name, uint8_t pin, const __FlashStringHelper *bleIdentifier)
    {
        this->index = index;
        this->name = name;
        this->pin = pin;
        this->bleIdentifier = bleIdentifier;
    }
};

class Led : public Device
{
public:
    uint8_t pin = 0;
    bool on = false;

    Led(uint8_t index, const __FlashStringHelper *name, uint8_t pin, const __FlashStringHelper *bleIdentifier)
    {
        this->index = index;
        this->name = name;
        this->pin = pin;
        this->bleIdentifier = bleIdentifier;
    }
};

class DeviceHandlerClass
{

private:
    Adafruit_seesaw seesaw;
    uint16_t pid;
    uint8_t year, mon, day;
    bool seesawReady = false;
    ButtonChangeCallback buttonChangeCallback;

    bool startSeesaw();

public:
    DeviceHandlerClass();
    void tick();
    void begin(ButtonChangeCallback buttonChangeCallback);
    void turnOnLED(uint8_t number);
    void turnOffLED(uint8_t number);
    // Map buttons to their index, I/O Pin, and BLE id.
    Button buttons[4] = {{0, F("Button 1"), 18, F("6E400002-B5A3-F393-E0A9-E50E24DCCA9E")}, {1, F("Button 2"), 19, F("6E400003-B5A3-F393-E0A9-E50E24DCCA9E")}, {2, F("Button 3"), 20, F("6E400004-B5A3-F393-E0A9-E50E24DCCA9E")}, {3, F("Button 4"), 2, F("6E400005-B5A3-F393-E0A9-E50E24DCCA9E")}};
    // Map led to their index, I/O Pin, and BLE id.
    Led leds[4] = {{0, F("Led 1"), 12, F("6E400006-B5A3-F393-E0A9-E50E24DCCA9E")}, {1, F("Led 2"), 13, F("6E400007-B5A3-F393-E0A9-E50E24DCCA9E")}, {2, F("Led 3"), 0, F("6E400008-B5A3-F393-E0A9-E50E24DCCA9E")}, {3, F("Led 4"), 1, F("6E400009-B5A3-F393-E0A9-E50E24DCCA9E")}};
};

extern DeviceHandlerClass DeviceHandler;
