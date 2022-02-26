
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



const char btDeviceName[] = "BLEButtons";

// Scheduler
Scheduler scheduler;
Task blinkTask;
Task devicesTask;
Task protocolTask;


Blinker blinker;
Devices devices(&deviceButtonCallback);
ProtocolProcessor protocolProcessor(&protocolLedCallback);


BluetoothSerial serialBT;


void setup()
{

  Serial.begin(115200);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  serialBT.begin(btDeviceName);
  protocolProcessor.begin(&serialBT);
  
  Log.noticeln(F("The device %s started, now you can pair it with bluetooth!"), btDeviceName);

  devices.begin();

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

void protocolTick() {
  protocolProcessor.tick();
}

void deviceButtonCallback(uint8_t buttonNumber, bool isOn)
{
  Log.traceln(F("Got button call back"));
  if (isOn)
    devices.turnOnLED(buttonNumber);
  else
    devices.turnOffLED(buttonNumber);
}

void protocolLedCallback(uint8_t ledNumber, bool turnOn)
{
   Log.traceln(F("Got protocol call back"));
  if (turnOn)
    devices.turnOnLED(ledNumber);
  else
    devices.turnOffLED(ledNumber);
}
