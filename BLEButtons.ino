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
#include "SoundsPlayer.h"
#include "BLEHandler.h"

#define VS1053_RESET -1 // VS1053 reset pin (not used!)
#define VS1053_CS 32    // VS1053 chip select pin (output)
#define VS1053_DCS 33   // VS1053 Data/command select pin (output)
#define CARDCS 14       // Card chip select pin
#define VS1053_DREQ 15  // VS1053 Data request, ideally an Interrupt pin

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
  BLEHandler.begin(&receivedMessageCallback);
  Log.noticeln(F("The device %s started, now you can pair it with bluetooth!"), BLEHandler.deviceName);

  //  if (!soundPlayer.begin())
  //  {
  //    Log.errorln(F("Couldn't initialize VS1053"));
  //  }
  // soundPlayer.setVolume(0, 0);

  // if (!SD.begin(CARDCS))
  //  {
  //    Log.errorln(F("Couldn't initialize SD card"));
  //  }

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
void receivedMessageCallback(const char* message, size_t size)
{

}