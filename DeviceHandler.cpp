#include "DeviceHandler.h"
#include <ArduinoLog.h>

DeviceHandlerClass::DeviceHandlerClass()
{
}

void DeviceHandlerClass::begin(ButtonChangeCallback buttonChangeCallback)
{
    for(Button &btn : buttons) {
        btn.deviceHandler = this;
    }

    for(Led &led : leds) {
        led.deviceHandler = this;
    }

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

        for (Button &btn : buttons)
        {

            switchOn = !seesaw.digitalRead(btn.pin);

            if (switchOn)
            {
                if (!btn.debounce)
                {
                    Log.traceln(F("[DeviceHandler] Button %d on"), btn.index + 1);
                    btn.debounce = true;
                    btn.notify(true);
                    buttonChangeCallback(btn.index,true);

                }
            }
            else if (!switchOn)
            {
                if (btn.debounce)
                {
                    Log.traceln(F("[DeviceHandler] Button %d off"), btn.index + 1);
                    btn.debounce = false;
                    btn.notify(false);
                    buttonChangeCallback(btn.index,false);

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

            for (Button &btn : buttons)
            {
                seesaw.pinMode(btn.pin, INPUT_PULLUP);
            }

            for (Led &led : leds)
            {
                turnOffLED(led.index);
            }

            seesawReady = true;
        }
    }

    return seesawReady;
}

void DeviceHandlerClass::turnOnLED(uint8_t index)
{
    if (index < sizeof(leds) && seesawReady) {
        seesaw.analogWrite(leds[index].pin, 255);
        leds[index].notify(true);
    }
}
void DeviceHandlerClass::turnOffLED(uint8_t index)
{
    if (index < sizeof(leds) && seesawReady) {
        seesaw.analogWrite(leds[index].pin, 0);
        leds[index].notify(false);
    }
}

DeviceHandlerClass DeviceHandler;