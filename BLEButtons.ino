#include <BluetoothSerial.h>
#include <Adafruit_seesaw.h>
#include <ArduinoLog.h>
#include <TaskScheduler.h>


#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#include "Devices.h"
#include "Blinker.h"
#include "ProtocolProcessor.h"
#include "Settings.h"
#include "WifiHandler.h"

const char btDeviceName[] = "BLEButtons";

// Scheduler
Scheduler scheduler;
Task blinkTask;
Task devicesTask;
Task protocolTask;
Task wifiTask;

Blinker blinker;
Devices devices(&deviceButtonCallback);
ProtocolProcessor protocolProcessor(&protocolLedCallback,&protocolReportCallback);

BluetoothSerial serialBT;

void setup()
{

  // Start serial logging
  Serial.begin(115200);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  Settings.being();

  // Start bluetooth and the protocol processor that wull send and receive commands
  serialBT.begin(btDeviceName);
  protocolProcessor.begin(&serialBT);

  Log.noticeln(F("The device %s started, now you can pair it with bluetooth!"), btDeviceName);


  devices.begin();

  WifiHandler.connect();

 
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

void deviceButtonCallback(uint8_t buttonNumber, bool isOn)
{
  Log.traceln(F("Got button callback for button %d"), buttonNumber);
  if (isOn)
  {
    protocolProcessor.sendButtonPress(buttonNumber);
    devices.turnOnLED(buttonNumber);
  }
  else
  {
    devices.turnOffLED(buttonNumber);
  }
}

void protocolLedCallback(uint8_t ledNumber, bool turnOn)
{
  Log.traceln(F("Got protocol callback for led %d"), ledNumber);
  if (turnOn)
    devices.turnOnLED(ledNumber);
  else
    devices.turnOffLED(ledNumber);
}

void protocolReportCallback()
{
  Log.traceln(F("Got protocol callback for reports"));
  DynamicJsonDocument jsonDocument(250);

  Settings.report(jsonDocument);

  protocolProcessor.sendReport(jsonDocument);


}
