#include <Adafruit_seesaw.h>
#include <ArduinoLog.h>
#include <SPI.h>
#include <SD.h>
#include <TaskScheduler.h>


#include "DeviceHandler.h"
#include "BlinkerHandler.h"
#include "ProtocolProcessor.h"
#include "SettingHandler.h"
#include "WifiHandler.h"
#include "ESPBattery.h"
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
Task protocolTask;
Task wifiTask;
Task batteryTask;

// ProtocolProcessor protocolProcessor(&protocolLedCallback, &protocolReportCallback, &protocolPlayCallback);
// ESPBattery battery(&batteryCallback);
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

  // protocolProcessor.begin(&serialBT);
  //  Log.noticeln(F("The device %s started, now you can pair it with bluetooth!"), btDeviceName);

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

  // Devices Task
  // devicesTask.set(TASK_MILLISECOND * 40, TASK_FOREVER, &devicesTick);
  // scheduler.addTask(devicesTask);
  // devicesTask.enable();

  // Protocol Task
  // protocolTask.set(TASK_MILLISECOND * 40, TASK_FOREVER, &protocolTick);
  // scheduler.addTask(protocolTask);
  // protocolTask.enable();

  // WiFi Task
  // wifiTask.set(TASK_MILLISECOND * 60000 * 5, TASK_FOREVER, &wifiTick);
  // scheduler.addTask(wifiTask);
  // wifiTask.enable();

  // batteryTask.set(TASK_MILLISECOND * 60000 * 2, TASK_FOREVER, &batteryTick);
  // scheduler.addTask(batteryTask);
  // batteryTask.enable();
}

void loop()
{
  scheduler.execute();
}

void btWriteMessage(String *message)
{
  // serialBT.write((const uint8_t *)message->c_str(), message->length());
  // serialBT.write('\n');
}

void btWriteButtonMessage(int number)
{
  // serialBT.write('b');
  // serialBT.write('0' + number);
  // serialBT.write('\n');
}

void blinkTick()
{
  BlinkerHandler.tick();
}

void devicesTick()
{
  DeviceHandler.tick();
}

void protocolTick()
{
  // protocolProcessor.tick();
}

void wifiTick()
{
  WifiHandler.tick();
}

void batteryTick()
{
  // battery.tick();
}

void deviceButtonCallback(uint8_t buttonNumber, bool isOn)
{
  Log.traceln(F("Got button callback for button %d"), buttonNumber);
  if (isOn)
  {
    // protocolProcessor.sendStatus("button", "on", buttonNumber);
    DeviceHandler.turnOnLED(buttonNumber);
    // protocolProcessor.sendStatus("led", "on", buttonNumber);
  }
  else
  {
    // protocolProcessor.sendStatus("button", "off", buttonNumber);
    DeviceHandler.turnOffLED(buttonNumber);
    // protocolProcessor.sendStatus("led", "off", buttonNumber);
  }
}

void protocolLedCallback(uint8_t ledNumber, bool turnOn)
{
  Log.traceln(F("Got protocol callback for led %d"), ledNumber);
  if (turnOn)
  {
    DeviceHandler.turnOnLED(ledNumber);
    // protocolProcessor.sendStatus("led", "on", ledNumber, true);
  }
  else
  {
    DeviceHandler.turnOffLED(ledNumber);
    // protocolProcessor.sendStatus("led", "off", ledNumber, true);
  }
}

void protocolReportCallback()
{
  Log.traceln(F("Got protocol callback for reports"));
  DynamicJsonDocument jsonDocument(500);

  SettingHandler.report(jsonDocument);
  WifiHandler.report(jsonDocument);
  // battery.report(jsonDocument);

  jsonDocument[F("memory")][F("totaheap")] = ESP.getHeapSize();
  jsonDocument[F("memory")][F("freeheap")] = ESP.getFreeHeap();
  jsonDocument[F("memory")][F("totalpsram")] = ESP.getPsramSize();
  jsonDocument[F("memory")][F("freepsram")] = ESP.getFreePsram();
  jsonDocument[F("memory")][F("flashchipsize")] = ESP.getFlashChipSize();

  // protocolProcessor.send(jsonDocument);
}

void batteryCallback(ESPBattery &b)
{
  int state = b.getState();
  int level = b.getLevel();
  float voltage = b.getVoltage();

  Log.traceln(F("Current state: $s"), b.stateToString(state));
  Log.traceln(F("Current level: %d "), level);

  Log.traceln(F("Current voltage: %f"), voltage);
}

void protocolPlayCallback(const char *filename)
{
  Log.traceln(F("Play: %s"), filename);
  // soundPlayer.play(filename);
  // protocolProcessor.sendStatus("sound", "play", filename, true);
}
