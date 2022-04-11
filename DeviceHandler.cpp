#include "DeviceHandler.h"
#include <ArduinoLog.h>

struct button
{
    uint8_t index = 0;
    uint8_t pin = 0;
    bool debounce = true;

    button(uint8_t index, uint8_t pin)
    {
        this->index = index;
        this->pin = pin;
    }
};
button buttons[] = {{0, 18}, {1, 19}, {2, 20}, {3, 2}};
struct led
{
    uint8_t index = 0;
    uint8_t pin = 0;
    bool on = false;
    led(uint8_t index, uint8_t pin)
    {
        this->index = index;
        this->pin = pin;
    }
};
led leds[] = {{0, 12}, {1, 13}, {2, 0}, {3, 1}};

DeviceHandlerClass::DeviceHandlerClass()
{
}

void DeviceHandlerClass::begin(ButtonChangeCallback buttonChangeCallback)
{
    this->buttonChangeCallback = buttonChangeCallback;

    // Turn off LDO2 to cut power to seesaw to reset
    Log.traceln(F("[DeviceHandler] Turning off LDO2 to reset I2C devices"));
    pinMode(LDO2, OUTPUT);
    digitalWrite(LDO2, 0);
    delay(2000);

    Log.traceln(F("[DeviceHandler] Turning on LDO2"));
    // Turn on LDO2 to give seesaw power
    digitalWrite(LDO2, 255);
    delay(2000);

    // Starter up
    startSeesaw();
}

void DeviceHandlerClass::tick()
{
    if (seesawReady)
    {
        bool switchOn = false;

        for (button &btn : buttons)
        {

            switchOn = !seesaw.digitalRead(btn.pin);

            if (switchOn)
            {
                if (!btn.debounce)
                {
                    Log.traceln(F("[DeviceHandler] Button %d on"), btn.index + 1);
                    btn.debounce = true;

                    if (buttonChangeCallback != NULL)
                        buttonChangeCallback(btn.index + 1, true);
                }
            }
            else if (!switchOn)
            {
                if (btn.debounce)
                {
                    Log.traceln(F("[DeviceHandler] Button %d off"), btn.index + 1);
                    btn.debounce = false;

                    if (buttonChangeCallback != NULL)
                        buttonChangeCallback(btn.index + 1, false);
                }
            }
        }
    }
}

bool DeviceHandlerClass::startSeesaw()
{
    if (!seesaw.begin(DEFAULT_I2C_ADDR))
    {
        Log.errorln(F("[DeviceHandler] Seesaw not found!"));
        seesawReady = false;
    }
    else
    {

        seesaw.getProdDatecode(&pid, &year, &mon, &day);
        Log.noticeln(F("[DeviceHandler] Seesaw found PID: %d datecode: %d/%d/%d"), pid, 2000 + year, mon, day);

        if (pid != 5296)
        {
            Log.errorln(F("[DeviceHandler] Wrong seesaw PID"));
            seesawReady = false;
        }
        else
        {

            Log.noticeln(F("[DeviceHandler] Seesaw started OK!"));

            for (button btn : buttons)
            {
                seesaw.pinMode(btn.pin, INPUT_PULLUP);
            }

            for (led l : leds)
            {
                seesaw.analogWrite(l.pin, 127);
            }

            seesawReady = true;
        }
    }

    return seesawReady;
}

void DeviceHandlerClass::turnOnLED(uint8_t number)
{
    uint8_t index = number - 1;
    if (index >= 0 && number < sizeof(leds) && seesawReady)
        seesaw.analogWrite(leds[index].pin, 127);
}
void DeviceHandlerClass::turnOffLED(uint8_t number)
{
    uint8_t index = number - 1;
    if (index >= 0 && number < sizeof(leds) && seesawReady)
        seesaw.analogWrite(leds[index].pin, 0);
}

DeviceHandlerClass DeviceHandler;