
#include "BatteryHandler.h"
#include "ArduinoLog.h"

BatteryHandlerClass::BatteryHandlerClass()
{
  min_level = ESPBATTERY_CRITICAL;
  max_level = ESPBATTERY_FULL;
  readData();
}

void BatteryHandlerClass::readData()
{
  pinMode(VBUS_SENSE, INPUT);
  adcAttachPin(VBAT_SENSE);
  analogSetPinAttenuation(VBAT_SENSE, ADC_11db); 

  vbusVoltage = digitalRead(VBUS_SENSE);
  batteryVoltage = analogReadMilliVolts(VBAT_SENSE);
  percentage = map(batteryVoltage, min_level, max_level, 0, 100);

  if (percentage < 0)
  {
    percentage = 0;
  }
  else if (percentage > 100)
  {
    percentage = 100;
  }
  getState();
}

void BatteryHandlerClass::tick()
{
  readData();
}

float BatteryHandlerClass::getVoltage()
{
  return batteryVoltage;
}

int BatteryHandlerClass::getPercentage()
{
  return percentage;
}

int BatteryHandlerClass::getState()
{
  last_state = state;
  if (batteryVoltage >= ESPBATTERY_CHARGING)
  {
    state = ESPBATTERY_CHARGING;
  }
  else if (batteryVoltage >= ESPBATTERY_FULL)
  {
    state = ESPBATTERY_FULL;
  }
  else if (batteryVoltage <= ESPBATTERY_CRITICAL)
  {
    state = ESPBATTERY_CRITICAL;
  }
  else if (batteryVoltage <= ESPBATTERY_LOW)
  {
    state = ESPBATTERY_LOW;
  }
  else
  {
    state = ESPBATTERY_OK;
  }

  if (state != last_state)
  {
    if (changed_cb != NULL)
      changed_cb();
    switch (state)
    {
    case ESPBATTERY_CHARGING:
      if (charging_cb != NULL)
        charging_cb();
      break;
    case ESPBATTERY_CRITICAL:
      if (critical_cb != NULL)
        critical_cb();
      break;
    case ESPBATTERY_LOW:
      if (low_cb != NULL)
        low_cb();
      break;
    }
  }
  return state;
}

String BatteryHandlerClass::stateToString(int state)
{
  switch (state)
  {
  case ESPBATTERY_OK:
    return "Ok";
  case ESPBATTERY_FULL:
    return "Full";
  case ESPBATTERY_CHARGING:
    return "Charging";
  case ESPBATTERY_CRITICAL:
    return "Critical";
  case ESPBATTERY_LOW:
    return "Low";
  default:
    return "Unknown";
  }
}

int BatteryHandlerClass::getPreviousState()
{
  return last_state;
}

void BatteryHandlerClass::setLevelChangedHandler(CallbackFunction callbackFunction)
{
  changed_cb = callbackFunction;
}

void BatteryHandlerClass::setLevelLowHandler(CallbackFunction callbackFunction)
{
  low_cb = callbackFunction;
}

void BatteryHandlerClass::setLevelCriticalHandler(CallbackFunction callbackFunction)
{
  critical_cb = callbackFunction;
}

void BatteryHandlerClass::setLevelChargingHandler(CallbackFunction callbackFunction)
{
  charging_cb = callbackFunction;
}

void BatteryHandlerClass::setStateChangedChargingHandler(CallbackFunction callbackFunction)
{
  changed_cb = callbackFunction;
}

void BatteryHandlerClass::report(SpiRamJsonDocument &jsonDocument)
{
  readData();
  jsonDocument["battery"]["state"] = stateToString(state);
  jsonDocument["battery"]["vbusVoltage"] = vbusVoltage;
  jsonDocument["battery"]["batteryVoltage"] = batteryVoltage;
  jsonDocument["battery"]["percentage"] = percentage;
}


BatteryHandlerClass BatteryHandler;