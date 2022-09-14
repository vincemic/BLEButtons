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
    delay(1000);

    if (!SoundPlayer.play("/starting.mp3"))
      Log.errorln(F("Couldn't play starting.mp3"));
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
}

void blinkTick()
{
  BlinkerHandler.tick();
  InitializeFiles();
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

void deviceButtonCallback(Button &btn, bool isOn)
{
  Log.traceln(F("Got button callback for %s"), btn.name);

  if (btn.index < 4)
  {
    if (isOn)
    {
      DeviceHandler.turnOnLED(btn.index);
    }
    else
    {
      DeviceHandler.turnOffLED(btn.index);
    }
  }
  else
  {
    if (btn.index == 4)
    {
      if (isOn)
      {
        SoundPlayer.play("/sw0001on.mp3");
      }
      else
      {
        SoundPlayer.play("/sw0001of.mp3");
      }
    }
    if (btn.index == 7)
    {
      if (!isOn)
      {
        SoundPlayer.play("/subdives.mp3");
      }
    }
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
}

void batteryCallback()
{
  Log.traceln(F("Current state: %s"), BatteryHandler.stateToString(BatteryHandler.getState()));
  Log.traceln(F("Current level: %d "), BatteryHandler.getLevel());

  Log.traceln(F("Current voltage: %f"), BatteryHandler.getVoltage());
}

void InitializeFiles()
{
  bool reset = false;

  if (WifiHandler.isConnected() && isInitialized == false)
  {
    isInitialized = true;

    if (reset || !SD.exists("/starting.mp3"))
    {
      WifiHandler.createTTSFile("Welcome Vincent, I am getting the system ready, please wait", "starting");
      yield();
      WifiHandler.getFile("/starting.mp3");
      yield();
    }

    if (reset || !SD.exists("/systemup.mp3"))
    {
      WifiHandler.createTTSFile("Thank you for waiting, the command system is ready for you Sir", "systemup");
      yield();
      WifiHandler.getFile("/systemup.mp3");
      yield();
    }

    if (reset || !SD.exists("/sw0001on.mp3"))
    {
      WifiHandler.createTTSFile("Switch one is on", "sw0001on");
      yield();
      WifiHandler.getFile("/sw0001on.mp3");
      yield();
    }

    if (reset || !SD.exists("/sw0001of.mp3"))
    {
      WifiHandler.createTTSFile("Switch one is off", "sw0001of");
      yield();
      WifiHandler.getFile("/sw0001of.mp3");
      yield();
    }

    if (reset || !SD.exists("/voxsdone.mp3"))
    {
      WifiHandler.createTTSFile("Done creating voice files", "voxsdone");
      yield();
      WifiHandler.getFile("/voxsdone.mp3");
      yield();
    }

    if (reset || !SD.exists("/subdives.mp3"))
    {
      WifiHandler.getFile("/subdives.mp3");
      yield();
    }

    SoundPlayer.play("/systemup.mp3");
  }
}
