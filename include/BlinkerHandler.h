#pragma once
#include <Adafruit_NeoPixel.h>


class BlinkerHandlerClass
{
public:
	BlinkerHandlerClass();
	void tick();
	void begin();
	void neoGreem();
	void neoRed();
	void neoBlue();

private:
	void blink();
	bool _isOn = false;
	Adafruit_NeoPixel *pixel;
	uint32_t color = 0;
};

extern BlinkerHandlerClass BlinkerHandler;

