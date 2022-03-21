#include <BluetoothSerial.h>
#include <Adafruit_seesaw.h>
#include <ArduinoLog.h>
#include <SPI.h>
#include <SD.h>

#include <TaskScheduler.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#include "Devices.h"
#include "Blinker.h"
#include "ProtocolProcessor.h"
#include "Settings.h"
#include "WifiHandler.h"
#include "ESPBattery.h"
#include "SoundsPlayer.h"

const char btDeviceName[] = "BLEButtons";

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

Blinker blinker;
Devices devices(&deviceButtonCallback);
ProtocolProcessor protocolProcessor(&protocolLedCallback, &protocolReportCallback, &protocolPlayCallback);
ESPBattery battery(&batteryCallback);
SoundPlayer soundPlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);

BluetoothSerial serialBT;

void setup()
{

  // Start serial logging
  Serial.begin(115200);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial, false);
  Settings.being();
  devices.begin();

  if (!serialBT.begin(btDeviceName))
  {
    Log.errorln(F("Couldn't initialize Bluetooth"));
  }

  protocolProcessor.begin(&serialBT);
  Log.noticeln(F("The device %s started, now you can pair it with bluetooth!"), btDeviceName);

  if (!soundPlayer.begin())
  {
    Log.errorln(F("Couldn't initialize VS1053"));
  }
  soundPlayer.setVolume(0, 0);

  if (!SD.begin(CARDCS))
  {
    Log.errorln(F("Couldn't initialize SD card"));
  }

  // WifiHandler.connect();

  // Blink Task
  blinkTask.set(TASK_MILLISECOND * 1000, TASK_FOREVER, &blinkTick);
  scheduler.addTask(blinkTask);
  blinkTask.enable();

  // Devices Task
  devicesTask.set(TASK_MILLISECOND * 40, TASK_FOREVER, &devicesTick);
  scheduler.addTask(devicesTask);
  devicesTask.enable();

  // Protocol Task
  protocolTask.set(TASK_MILLISECOND * 40, TASK_FOREVER, &protocolTick);
  scheduler.addTask(protocolTask);
  protocolTask.enable();

  // WiFi Task
  wifiTask.set(TASK_MILLISECOND * 60000 * 5, TASK_FOREVER, &wifiTick);
  scheduler.addTask(wifiTask);
  wifiTask.enable();

  batteryTask.set(TASK_MILLISECOND * 60000 * 1, TASK_FOREVER, &batteryTick);
  scheduler.addTask(batteryTask);
  batteryTask.enable();
}

void loop()
{
  scheduler.execute();
}

void btWriteMessage(String *message)
{
  serialBT.write((const uint8_t *)message->c_str(), message->length());
  serialBT.write('\n');
}

void btWriteButtonMessage(int number)
{
  serialBT.write('b');
  serialBT.write('0' + number);
  serialBT.write('\n');
}

void blinkTick()
{
  blinker.tick();
}

void devicesTick()
{
  devices.tick();
}

void protocolTick()
{
  protocolProcessor.tick();
}

void wifiTick()
{
  WifiHandler.tick();
}

void batteryTick()
{
  battery.tick();
}

void deviceButtonCallback(uint8_t buttonNumber, bool isOn)
{
  Log.traceln(F("Got button callback for button %d"), buttonNumber);
  if (isOn)
  {
    protocolProcessor.sendStatus("button", "on", buttonNumber);
    devices.turnOnLED(buttonNumber);
    protocolProcessor.sendStatus("led", "on", buttonNumber);
  }
  else
  {
    protocolProcessor.sendStatus("button", "off", buttonNumber);
    devices.turnOffLED(buttonNumber);
    protocolProcessor.sendStatus("led", "off", buttonNumber);
  }
}

void protocolLedCallback(uint8_t ledNumber, bool turnOn)
{
  Log.traceln(F("Got protocol callback for led %d"), ledNumber);
  if (turnOn)
  {
    devices.turnOnLED(ledNumber);
    protocolProcessor.sendStatus("led", "on", ledNumber, true);
  }
  else
  {
    devices.turnOffLED(ledNumber);
    protocolProcessor.sendStatus("led", "off", ledNumber, true);
  }
}

void protocolReportCallback()
{
  Log.traceln(F("Got protocol callback for reports"));
  DynamicJsonDocument jsonDocument(550);

  Settings.report(jsonDocument);
  WifiHandler.report(jsonDocument);
  battery.report(jsonDocument);

  protocolProcessor.send(jsonDocument);
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
  soundPlayer.play(filename);
}
