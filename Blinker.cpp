#include "Blinker.h"
#include "Arduino.h"

Blinker::Blinker()
{
	pinMode(LED_BUILTIN, OUTPUT);
}

void Blinker::tick()
{
	blink();
}

void Blinker::blink()
{
	if (_isOn)
	{
		digitalWrite(LED_BUILTIN, LOW);
		_isOn = false;
	}
	else
	{
		digitalWrite(LED_BUILTIN, HIGH);
		_isOn = true;
	}
}

