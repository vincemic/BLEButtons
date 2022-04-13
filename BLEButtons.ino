#include <Adafruit_seesaw.h>
#include <ArduinoLog.h>
#include <SPI.h>
#include <SD.h>
#include <TaskScheduler.h>

#include "DeviceHandler.h"
#include "BlinkerHandler.h"
#include "SettingHandler.h"
#include "WifiHandler.h"
#include "BatteryHandler.h"
#include "SoundPlayer.h"
#include "BLEHandler.h"

// Scheduler
Scheduler scheduler;
Task blinkTask;
Task devicesTask;
Task wifiTask;
Task batteryTask;
Task bleTask;

// SoundPlayer soundPlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);

void setup()
{

  // Start serial logging
  Serial.begin(115200);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial, false);

  delay(10000);

  BlinkerHandler.begin();
  SettingHandler.being();
  DeviceHandler.begin(&deviceButtonCallback);
  BLEHandler.begin();
  Log.noticeln(F("The device %s started, now you can pair it with bluetooth!"), BLEHandler.deviceName);

  if (!SoundPlayer.begin())
  {
    Log.errorln(F("Couldn't initialize VS1053"));
  }
  else
  {
    SoundPlayer.setVolume(0, 0);
  }


  WifiHandler.connect();

  // Blink Task
  blinkTask.set(TASK_MILLISECOND * 1000, TASK_FOREVER, &blinkTick);
  scheduler.addTask(blinkTask);
  blinkTask.enable();

  // BLE Task
  bleTask.set(TASK_MILLISECOND * 1000, TASK_FOREVER, &bleTick);
  scheduler.addTask(bleTask);
  bleTask.enable();

  // Devices Task
  devicesTask.set(TASK_MILLISECOND * 40, TASK_FOREVER, &devicesTick);
  scheduler.addTask(devicesTask);
  devicesTask.enable();

  // WiFi Task
  wifiTask.set(TASK_MILLISECOND * 60000 * 5, TASK_FOREVER, &wifiTick);
  scheduler.addTask(wifiTask);
  wifiTask.enable();

  // Battery Task
  batteryTask.set(TASK_MILLISECOND * 60000 * 2, TASK_FOREVER, &batteryTick);
  scheduler.addTask(batteryTask);
  batteryTask.enable();
}

void loop()
{
  scheduler.execute();
  delay(10);
}

void blinkTick()
{
  BlinkerHandler.tick();
}

void devicesTick()
{
  DeviceHandler.tick();
}

void wifiTick()
{
  WifiHandler.tick();
}

void batteryTick()
{
  BatteryHandler.tick();
  protocolReportCallback();
}

void bleTick()
{
  BLEHandler.tick();
}

void deviceButtonCallback(uint8_t index, bool isOn)
{
  Log.traceln(F("Got button callback for button %d"), index + 1);
  if (isOn)
  {
    DeviceHandler.turnOnLED(index);
  }
  else
  {
    DeviceHandler.turnOffLED(index);
  }
}

void protocolReportCallback()
{
  Log.traceln(F("Got protocol callback for reports"));
  SpiRamJsonDocument jsonDocument(500);

  SettingHandler.report(jsonDocument);
  WifiHandler.report(jsonDocument);
  BatteryHandler.report(jsonDocument);

  jsonDocument[F("memory")][F("totaheap")] = ESP.getHeapSize();
  jsonDocument[F("memory")][F("freeheap")] = ESP.getFreeHeap();
  jsonDocument[F("memory")][F("totalpsram")] = ESP.getPsramSize();
  jsonDocument[F("memory")][F("freepsram")] = ESP.getFreePsram();
  jsonDocument[F("memory")][F("flashchipsize")] = ESP.getFlashChipSize();

  String json;
  serializeJson(jsonDocument, json);
  Log.traceln(F("Report:\r%s"), json.c_str());
}

void batteryCallback()
{
  Log.traceln(F("Current state: %s"), BatteryHandler.stateToString(BatteryHandler.getState()));
  Log.traceln(F("Current level: %d "), BatteryHandler.getLevel());

  Log.traceln(F("Current voltage: %f"), BatteryHandler.getVoltage());
}

void protocolPlayCallback(const char *filename)
{
  Log.traceln(F("Play: %s"), filename);
  // soundPlayer.play(filename);
}
