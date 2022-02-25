#pragma once



class Blinker
{
public:
	Blinker();

	void tick();

private:
	void blink();
	bool _isOn = false;
};



