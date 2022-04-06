
#include "ESPBattery.h"
#include "ArduinoLog.h"

ESPBattery::ESPBattery(CallbackFunction stateChangedCallback)
{
  pin = A12;
  min_level = ESPBATTERY_CRITICAL;
  max_level = ESPBATTERY_FULL;
  changed_cb = stateChangedCallback;
  readData();
}

void ESPBattery::readData()
{
  level = analogRead(pin) - 279;
  voltage = ((float)level * 2 / 1000);
  percentage = map(level, min_level, max_level, 0, 100);

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

void ESPBattery::tick()
{
  readData();
}

float ESPBattery::getVoltage()
{
  return voltage;
}

int ESPBattery::getPercentage()
{
  return percentage;
}

int ESPBattery::getLevel()
{
  return level;
}

int ESPBattery::getState()
{
  last_state = state;
  if (level >= ESPBATTERY_CHARGING)
  {
    state = ESPBATTERY_CHARGING;
  }
  else if (level >= ESPBATTERY_FULL)
  {
    state = ESPBATTERY_FULL;
  }
  else if (level <= ESPBATTERY_CRITICAL)
  {
    state = ESPBATTERY_CRITICAL;
  }
  else if (level <= ESPBATTERY_LOW)
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
      changed_cb(*this);
    switch (state)
    {
    case ESPBATTERY_CHARGING:
      if (charging_cb != NULL)
        charging_cb(*this);
      break;
    case ESPBATTERY_CRITICAL:
      if (critical_cb != NULL)
        critical_cb(*this);
      break;
    case ESPBATTERY_LOW:
      if (low_cb != NULL)
        low_cb(*this);
      break;
    }
  }
  return state;
}

String ESPBattery::stateToString(int state)
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

int ESPBattery::getPreviousState()
{
  return last_state;
}

void ESPBattery::setLevelChangedHandler(CallbackFunction f)
{
  changed_cb = f;
}

void ESPBattery::setLevelLowHandler(CallbackFunction f)
{
  low_cb = f;
}

void ESPBattery::setLevelCriticalHandler(CallbackFunction f)
{
  critical_cb = f;
}

void ESPBattery::setLevelChargingHandler(CallbackFunction f)
{
  charging_cb = f;
}

void ESPBattery::report(JsonDocument &jsonDocument)
{
  readData();
  jsonDocument["battery"]["state"] = stateToString(state);
  jsonDocument["battery"]["level"] = level;
  jsonDocument["battery"]["voltage"] = voltage;
  jsonDocument["battery"]["percentage"] = percentage;
}
