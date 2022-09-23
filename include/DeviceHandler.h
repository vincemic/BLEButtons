#pragma once
#include <ArduinoLog.h>
#include <Adafruit_seesaw.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include "SettingHandler.h"
#include "SoundPlayer.h"

#define SEESAW1_I2C_ADDR 0x3A
#define SEESAW2_I2C_ADDR 0x3B



class DeviceHandlerBase
{
public:
    virtual void turnOnLED(uint8_t index) = 0;
    virtual void turnOffLED(uint8_t index) = 0;
};

class Device : public BLECharacteristicCallbacks
{
public:
    DeviceHandlerBase *deviceHandler;
    const char *bleIdentifier;
    const char *name;
    uint8_t index = 0;
    BLECharacteristic *pCharacteristic;

    virtual uint32_t getProperties() = 0;
};

class Button : public Device
{
public:
    uint8_t pin = 0;
    bool debounce = true;

    Button(uint8_t index, const __FlashStringHelper *name, uint8_t pin, const __FlashStringHelper *bleIdentifier)
    {
        this->index = index;
        this->name = (const char *)name;
        this->pin = pin;
        this->bleIdentifier = (const char *)bleIdentifier;
    }

    uint32_t getProperties()
    {
        return BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ;
    }

    void notify(bool isOn)
    {
        if (isOn)
            pCharacteristic->setValue("on");
        else
            pCharacteristic->setValue("off");

        pCharacteristic->notify();
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
        this->name = (const char *)name;
        this->pin = pin;
        this->bleIdentifier = (const char *)bleIdentifier;
    }

    uint32_t getProperties()
    {
        return BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ;
    }

    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string value = pCharacteristic->getValue();

        if (value == "on")
            deviceHandler->turnOnLED(index);
        else
            deviceHandler->turnOffLED(index);
    }

    void notify(bool isOn)
    {
        if (isOn)
            pCharacteristic->setValue("on");
        else
            pCharacteristic->setValue("off");

        pCharacteristic->notify();
    }
};

class Setting : public Device
{
public:
    const char *label;
    bool allowRead = true;

    Setting(uint8_t index, const __FlashStringHelper *name, const char *label, bool allowRead, const __FlashStringHelper *bleIdentifier)
    {
        this->index = index;
        this->name = (const char *)name;
        this->label = label;
        this->bleIdentifier = (const char *)bleIdentifier;
        this->allowRead = allowRead;
    }

    uint32_t getProperties()
    {
        if (allowRead)
            return BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ;
        else
            return BLECharacteristic::PROPERTY_WRITE;
    }

    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string value = pCharacteristic->getValue();
        SettingHandler.write(label, value.c_str());
    }

    void onRead(BLECharacteristic *pCharacteristic)
    {
        if (allowRead)
        {
            std::string value;
            SettingHandler.read(label, value);
            pCharacteristic->setValue(value.c_str());
        }
    }
};

class Player : public Device
{
public:
    Player(uint8_t index, const __FlashStringHelper *name, const __FlashStringHelper *bleIdentifier)
    {
        this->index = index;
        this->name = (const char *)name;
        this->bleIdentifier = (const char *)bleIdentifier;
    }

    uint32_t getProperties()
    {
        return BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY;
    }

    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string value = pCharacteristic->getValue();
        SoundPlayer.play(value.c_str());
    }

    void onRead(BLECharacteristic *pCharacteristic)
    {
        std::string value;
        // SettingHandler.read(label, value);
        pCharacteristic->setValue(value.c_str());
    }

    void notify(const char *filepath)
    {
        pCharacteristic->setValue(filepath);
        pCharacteristic->notify();
    }
};

typedef void (*ButtonChangeCallback)(Button &btn, bool isOn);

class DeviceHandlerClass : public DeviceHandlerBase
{

private:
    Adafruit_seesaw seesaw1;
    Adafruit_seesaw seesaw2;
    uint16_t pid;
    uint8_t year, mon, day;
    bool seesawsReady = false;
    ButtonChangeCallback buttonChangeCallback;

    bool startSeesaws();
    bool startSeesaw(Adafruit_seesaw &seesaw,uint8_t address);

public:
    DeviceHandlerClass();
    void tick();
    void begin(ButtonChangeCallback buttonChangeCallback);
    void turnOnLED(uint8_t index);
    void turnOffLED(uint8_t index);

    // Map buttons to their index, I/O Pin, and BLE id.
    Button buttons[8] = {{0, F("Button 1"), 18, F("6E400002-B5A3-F393-E0A9-E50E24DCCA9E")}, {1, F("Button 2"), 19, F("6E400003-B5A3-F393-E0A9-E50E24DCCA9E")}, {2, F("Button 3"), 20, F("6E400004-B5A3-F393-E0A9-E50E24DCCA9E")}, {3, F("Button 4"), 2, F("6E400005-B5A3-F393-E0A9-E50E24DCCA9E")},{4, F("Switch 1"), 18, F("6E430002-B5A3-F393-E0A9-E50E24DCCA9E")}, {5, F("Switch 2"), 19, F("6E430003-B5A3-F393-E0A9-E50E24DCCA9E")}, {6, F("Switch 3"), 20, F("6E430004-B5A3-F393-E0A9-E50E24DCCA9E")}, {7, F("Switch 4"), 2, F("6E430005-B5A3-F393-E0A9-E50E24DCCA9E")}};
    // Map led to their index, I/O Pin, and BLE id.
    Led leds[4] = {{0, F("Led 1"), 12, F("6E400006-B5A3-F393-E0A9-E50E24DCCA9E")}, {1, F("Led 2"), 13, F("6E400007-B5A3-F393-E0A9-E50E24DCCA9E")}, {2, F("Led 3"), 0, F("6E400008-B5A3-F393-E0A9-E50E24DCCA9E")}, {3, F("Led 4"), 1, F("6E400009-B5A3-F393-E0A9-E50E24DCCA9E")}};
    Setting settings[2] = {{0, F("WiFi SID"), WIFISID, true, F("6E410001-B5A3-F393-E0A9-E50E24DCCA9E")}, {1, F("WiFi Password"), WIFIPASSWORD, false, F("6E410002-B5A3-F393-E0A9-E50E24DCCA9E")}};
    Player players[1] = {{0, F("Sound Player"),F("6E420001-B5A3-F393-E0A9-E50E24DCCA9E")}};
};

extern DeviceHandlerClass DeviceHandler;
