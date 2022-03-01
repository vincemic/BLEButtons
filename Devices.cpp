#include "Devices.h"
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

Devices::Devices(ButtonChangeCallback buttonChangeCallback)
{
    this->buttonChangeCallback = buttonChangeCallback;
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
            if (!btn.debounce)
            {
                Log.traceln(F("Button %d on"), btn.index + 1);
                btn.debounce = true;

                if (buttonChangeCallback != NULL)
                    buttonChangeCallback(btn.index + 1, true);
            }
        }
        else if (!switchOn)
        {
            if (btn.debounce)
            {
                Log.traceln(F("Button %d off"), btn.index + 1);
                btn.debounce = false;

                if (buttonChangeCallback != NULL)
                    buttonChangeCallback(btn.index + 1, false);
            }
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

void Devices::turnOnLED(uint8_t number)
{
    uint8_t index = number - 1;
    if (index >= 0 && number < sizeof(leds))
        seesaw.analogWrite(leds[index].pin, 127);
}
void Devices::turnOffLED(uint8_t number)
{
    uint8_t index = number - 1;
    if (index >= 0 && number < sizeof(leds))
        seesaw.analogWrite(leds[index].pin, 0);
}