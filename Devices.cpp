#include "Devices.h"
#include <ArduinoLog.h>

struct button
{
    uint8_t index = 0;
    uint8_t pin = 0;
    uint8_t debounce = 0;
    bool on = false;
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

Devices::Devices()
{
}

void Devices::begin()
{

    startSeesaw();
}

void Devices::tick()
{
    bool switchOn = false;

    for (button &btn : buttons)
    {

        switchOn = !seesaw.digitalRead(btn.pin);

        if (switchOn)
        {
            seesaw.analogWrite(leds[btn.index].pin, 255);
            if (btn.debounce < 1)
            {
                Log.traceln(F("Button %d pressed "), btn.index +1);
                btn.debounce = 1;
                btn.on = true;
            }
        }
        else if (!switchOn)
        {
            seesaw.analogWrite(leds[btn.index].pin, 0);
            if (btn.debounce > 0)
                btn.debounce--;
        }
    }
}

bool Devices::startSeesaw()
{
    if (!seesaw.begin(DEFAULT_I2C_ADDR))
    {
        Log.errorln(F("seesaw not found!"));
        return false;
    }

    seesaw.getProdDatecode(&pid, &year, &mon, &day);
    Log.noticeln(F("seesaw found PID: %d datecode: %d/%d/%d"), pid, 2000 + year, mon, day);

    if (pid != 5296)
    {
        Log.errorln(F("Wrong seesaw PID"));
        return false;
    }

    Log.noticeln(F("seesaw started OK!"));

    for (button btn : buttons)
    {
        seesaw.pinMode(btn.pin, INPUT_PULLUP);
    }

    for (led l : leds)
    {
        seesaw.analogWrite(l.pin, 0);
    }

    return true;
}
