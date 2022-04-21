#include <ArduinoLog.h>
#include <TaskScheduler.h>


#include "DeviceHandler.h"
#include "BlinkerHandler.h"
#include "SettingHandler.h"
#include "WifiHandler.h"
#include "BatteryHandler.h"
#include "SoundPlayer.h"
#include "BLEHandler.h"
#include "RTCHandler.h"

// Scheduler
Scheduler scheduler;
Task blinkTask;
Task devicesTask;
Task wifiTask;
Task batteryTask;

bool isInitialized = false;

void setup()
{

  // Start serial logging
  Serial.begin(115200);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial, false);

  delay(3000);


  BlinkerHandler.begin();
  SettingHandler.being();
  DeviceHandler.begin(&deviceButtonCallback);
  BLEHandler.begin();
  RTCHandler.begin();

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

  // Devices Task
  devicesTask.set(TASK_MILLISECOND * 40, TASK_FOREVER, &devicesTick);
  scheduler.addTask(devicesTask);
  devicesTask.enable();

  // WiFi Task
  wifiTask.set(TASK_MILLISECOND * 60000 * 2, TASK_FOREVER, &wifiTick);
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

  jsonDocument[F("memory")][F("soundPlayerTaskHighWaterMark")] = uxTaskGetStackHighWaterMark(SoundPlayer.taskHandle);
  jsonDocument[F("memory")][F("settingHandlerTaskHighWaterMark")] = uxTaskGetStackHighWaterMark(SettingHandler.taskHandle);

  jsonDocument[F("memory")][F("totaHeap")] = ESP.getHeapSize();
  jsonDocument[F("memory")][F("freeHeap")] = ESP.getFreeHeap();
  jsonDocument[F("memory")][F("totalPSRAM")] = ESP.getPsramSize();
  jsonDocument[F("memory")][F("freePSRAM")] = ESP.getFreePsram();
  jsonDocument[F("memory")][F("flashChipSize")] = ESP.getFlashChipSize();

  String json;
  serializeJson(jsonDocument, json);
  Log.traceln(F("Report:\r%s"), json.c_str());

  InitializeFiles();
}

void batteryCallback()
{
  Log.traceln(F("Current state: %s"), BatteryHandler.stateToString(BatteryHandler.getState()));
  Log.traceln(F("Current level: %d "), BatteryHandler.getLevel());

  Log.traceln(F("Current voltage: %f"), BatteryHandler.getVoltage());
}

void InitializeFiles()
{
  if (WifiHandler.isConnected() && isInitialized == false)
  {
    isInitialized = true;
    WifiHandler.createTTSFile("Getting the system ready, please wait", "starting");
    yield();
    WifiHandler.getTTSFile("/starting.mp3");
    yield();
    SoundPlayer.play("/starting.mp3");
  }
}
