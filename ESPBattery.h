/////////////////////////////////////////////////////////////////
/*
    Arduino Library to calculate the ESP8266 (Feather Huzzah) LiPo battery level.
  Created by Lennart Hennigs, November 4, 2017.
*/
/////////////////////////////////////////////////////////////////

#pragma once

#ifndef ESPBattery_h
#define ESPBattery_h

/////////////////////////////////////////////////////////////////

#include <ArduinoJson.h>
#include "Arduino.h"

/////////////////////////////////////////////////////////////////

#define ESPBATTERY_CHARGING 2150 // lower boundary 4250
#define ESPBATTERY_FULL 2050     // lower boundary 4100
#define ESPBATTERY_OK 1975       // picked value 3950
#define ESPBATTERY_LOW 1900     // upper boundary 3800
#define ESPBATTERY_CRITICAL 1850 // upper boundary 3701

/////////////////////////////////////////////////////////////////

class ESPBattery
{
private:
  byte pin;
  unsigned int level;
  float voltage;
  int percentage;
  int min_level, max_level;
  int state, last_state;

  void readData();

  typedef void (*CallbackFunction)(ESPBattery &);

  CallbackFunction changed_cb = NULL;
  CallbackFunction low_cb = NULL;
  CallbackFunction critical_cb = NULL;
  CallbackFunction charging_cb = NULL;

public:
  ESPBattery(CallbackFunction f);

  float getVoltage();
  int getPercentage();
  int getLevel();
  int getState();
  int getPreviousState();
  String stateToString(int state);

  void setLevelChangedHandler(CallbackFunction f);
  void setLevelLowHandler(CallbackFunction f);
  void setLevelCriticalHandler(CallbackFunction f);
  void setLevelChargingHandler(CallbackFunction f);

  void tick();
  void report(JsonDocument &jsonDocument);
};

/////////////////////////////////////////////////////////////////
#endif
/////////////////////////////////////////////////////////////////


