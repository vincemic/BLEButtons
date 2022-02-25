#include "Devices.h"
#include <ArduinoLog.h>

struct button
{
    uint8_t index = 0;
    uint8_t pin = 0;
    uint8_t led = 0;
    uint8_t debounce = 0;
    button(uint8_t index, uint8_t pin, uint8_t led)
    {
        this->index = index;
        this->pin = pin;
        this->led = led;
    }
};
button buttons[] = {{1, 18, 12}, {2, 19, 13}, {3, 20, 0}, {4, 2, 1}};

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
            seesaw.analogWrite(btn.led, 255);
            if (btn.debounce < 1)
            {
                Log.traceln(F("Button %d pressed "), btn.index);
                btn.debounce = 1;
            }
        }
        else if (!switchOn)
        {
            seesaw.analogWrite(btn.led, 0);
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
    Log.noticeln(F("seesaw found PID: %d datecode: %d/%d/%d"), pid,2000 + year,mon, day);
  
    if (pid != 5296)
    {
        Log.errorln(F("Wrong seesaw PID"));
        return false;
    }

    Log.noticeln(F("seesaw started OK!"));

    for (button btn : buttons)
    {
        seesaw.pinMode(btn.pin, INPUT_PULLUP);
        seesaw.analogWrite(btn.led, 0);
    }

    return true;
}
