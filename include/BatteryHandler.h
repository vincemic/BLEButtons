
#pragma once

#include <ArduinoJson.h>
#include "Arduino.h"
#include "JsonHelper.h"

#define ESPBATTERY_CHARGING 2150 // lower boundary 4250
#define ESPBATTERY_FULL 2050     // lower boundary 4100
#define ESPBATTERY_OK 1975       // picked value 3950
#define ESPBATTERY_LOW 1900     // upper boundary 3800
#define ESPBATTERY_CRITICAL 1850 // upper boundary 3701


class BatteryHandlerClass
{
private:
  uint32_t batteryVoltage;
  int percentage;
  int min_level, max_level;
  int state, last_state;
  bool hasVbusPower = false;

  void readData();

  typedef void (*CallbackFunction)();

  CallbackFunction changed_cb = NULL;
  CallbackFunction low_cb = NULL;
  CallbackFunction critical_cb = NULL;
  CallbackFunction charging_cb = NULL;

public:
  BatteryHandlerClass();

  float getVoltage();
  int getPercentage();
  int getLevel();
  int getState();
  int getPreviousState();
  String stateToString(int state);

  void setLevelChangedHandler(CallbackFunction callbackFunction);
  void setLevelLowHandler(CallbackFunction callbackFunctionf);
  void setLevelCriticalHandler(CallbackFunction callbackFunction);
  void setLevelChargingHandler(CallbackFunction callbackFunction);
  void setStateChangedChargingHandler(CallbackFunction callbackFunction);

  void tick();
  void report(SpiRamJsonDocument &jsonDocument);
};

extern BatteryHandlerClass BatteryHandler;


