#include "DeviceHandler.h"
#include <ArduinoLog.h>

DeviceHandlerClass::DeviceHandlerClass()
{
}

void DeviceHandlerClass::begin(ButtonChangeCallback buttonChangeCallback)
{
    for (Button &btn : buttons)
    {
        btn.deviceHandler = this;
    }

    for (Led &led : leds)
    {
        led.deviceHandler = this;
    }

    this->buttonChangeCallback = buttonChangeCallback;

    // Turn off LDO2 to cut power to seesaw to reset
    Log.traceln(F("[DeviceHandler] Turning off LDO2 to reset I2C devices"));
    pinMode(LDO2, OUTPUT);
    digitalWrite(LDO2, 0);
    delay(1000);

    Log.traceln(F("[DeviceHandler] Turning on LDO2"));
    // Turn on LDO2 to give seesaw power
    digitalWrite(LDO2, 255);
    delay(1000);

    // Starter up
    seesawsReady = startSeesaws();
}

void DeviceHandlerClass::tick()
{
    if (seesawsReady)
    {
        bool switchOn = false;

        for (Button &btn : buttons)
        {
            if (btn.index < 4)
                switchOn = !seesaw1.digitalRead(btn.pin);
            else
                switchOn = !seesaw2.digitalRead(btn.pin);

            if (switchOn)
            {
                if (!btn.debounce)
                {
                    Log.traceln(F("[DeviceHandler] %s on"), btn.name);
                    btn.debounce = true;
                    btn.notify(true);
                    buttonChangeCallback(btn, true);
                }
            }
            else if (!switchOn)
            {
                if (btn.debounce)
                {
                    Log.traceln(F("[DeviceHandler] %s off"), btn.name);
                    btn.debounce = false;
                    btn.notify(false);
                    buttonChangeCallback(btn, false);
                }
            }
        }
    }
}

bool DeviceHandlerClass::startSeesaws()
{
    bool result = false;

    if (startSeesaw(seesaw1, SEESAW1_I2C_ADDR) && startSeesaw(seesaw2, SEESAW2_I2C_ADDR))
    {

        for (Button &btn : buttons)
        {
            if (btn.index < 4)
                seesaw1.pinMode(btn.pin, INPUT_PULLUP);
            else
                seesaw2.pinMode(btn.pin, INPUT_PULLUP);
        }

        for (Led &led : leds)
        {
            turnOffLED(led.index);
        }

        result = true;
    }

    return result;
}

bool DeviceHandlerClass::startSeesaw(Adafruit_seesaw &seesaw, uint8_t address)
{
    bool result = false;

    if (!seesaw.begin(address))
    {
        Log.errorln(F("[DeviceHandler] Seesaw at: %d not found!"), seesaw.getI2CAddr());
        result = false;
    }
    else
    {

        seesaw1.getProdDatecode(&pid, &year, &mon, &day);
        Log.noticeln(F("[DeviceHandler] Seesaw at: %d found PID: %d datecode: %d/%d/%d"), seesaw.getI2CAddr(), pid, 2000 + year, mon, day);

        if (pid != 5296)
        {
            Log.errorln(F("[DeviceHandler] Seesaw at: %d wrong PID"), seesaw.getI2CAddr());
            result = false;
        }
        else
        {

            Log.noticeln(F("[DeviceHandler] Seesaw at: %d  started OK!"), seesaw.getI2CAddr());
            result = true;
        }
    }

    return result;
}

void DeviceHandlerClass::turnOnLED(uint8_t index)
{
    if (index < sizeof(leds) && seesawsReady)
    {
        seesaw1.analogWrite(leds[index].pin, 255);
        leds[index].notify(true);
    }
}
void DeviceHandlerClass::turnOffLED(uint8_t index)
{
    if (index < sizeof(leds) && seesawsReady)
    {
        seesaw1.analogWrite(leds[index].pin, 0);
        leds[index].notify(false);
    }
}

DeviceHandlerClass DeviceHandler;