#include "BlinkerHandler.h"
#include <Arduino.h>
#include <ArduinoLog.h>


BlinkerHandlerClass::BlinkerHandlerClass() 
{

}

void BlinkerHandlerClass::begin() {

	Log.traceln(F("[Blinker] starting"));
	pinMode(LED_BUILTIN, OUTPUT);
	
	pixel = new Adafruit_NeoPixel(1, RGB_DATA, NEO_GRB + NEO_KHZ800);
	pixel->setPixelColor(0, pixel->Color(2, 2, 2));
    pixel->show();
}

void BlinkerHandlerClass::tick()
{
	blink();
}

void BlinkerHandlerClass::blink()
{
	if (_isOn)
	{
		digitalWrite(LED_BUILTIN, LOW);
		pixel->setPixelColor(0, color);
		 pixel->show();
		_isOn = false;
	}
	else
	{
		digitalWrite(LED_BUILTIN, HIGH);
		pixel->setPixelColor(0, pixel->Color(0, 0, 0));
		 pixel->show();
		_isOn = true;
	}
}

void BlinkerHandlerClass::neoBlue() {
	color = pixel->Color(0,0,127);
}

void BlinkerHandlerClass::neoGreem() {
	color = pixel->Color(0,127,0);
}

void BlinkerHandlerClass::neoRed() {
	color = pixel->Color(127,0,0);
}

BlinkerHandlerClass BlinkerHandler;
