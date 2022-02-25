#include <ArduinoJson.h>
#include <BluetoothSerial.h>
#include <Adafruit_seesaw.h>
#include <ArduinoLog.h>
#include <TaskScheduler.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#include "Devices.h"
#include "LoopbackStream.h"
#include "LEDCommand.h"
#include "Blinker.h"

#define BUFFER_SIZE 500
#define RETURN '\r'
#define LINEFEED '\n'

uint8_t inputCharacter;
uint8_t markerCount = 0;
const char btDeviceName[] = "BLEButtons";

// Scheduler
Scheduler scheduler;
Task blinkTask;
Task devicesTask;

// Blinker
Blinker blinker;

LoopbackStream protocolStream(BUFFER_SIZE);
DynamicJsonDocument jsonDocument(BUFFER_SIZE);
Devices devices;
BluetoothSerial serialBT;
Command commands[] = {LEDCommand(&devices)};

void setup()
{

  Serial.begin(115200);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);

  serialBT.begin(btDeviceName);
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
}

void loop()
{
  while (serialBT.available())
  {
    process(serialBT.read());
  }

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

void process(uint8_t character)
{

  if (character < 0)
    return;

  protocolStream.write(character);

  if (character == RETURN)
    markerCount++;
  else
    markerCount = 0;

  if (markerCount >= 2)
  {
    processDocument();
    markerCount = 0;
  }
}

void processDocument()
{
  DeserializationError error = deserializeJson(jsonDocument, protocolStream);
  protocolStream.clear();

  if (error)
  {
    Log.errorln(F("deserializeJson() failed: %s"), error.f_str());
    return;
  }
  else
  {

    logJsonDocument();
    Log.traceln(F("message received"));

    JsonVariant commandJson = jsonDocument["command"];

    if (commandJson.isNull())
    {
      Log.traceln(F("command not found"));
      return;
    }
    else
    {
      const char *commandName = commandJson.as<const char *>();
      Log.traceln(F("command found: %s"), commandName);

      for (Command &command : commands)
      {
        command.Execute(commandName, commandJson);
      }
    }
  }
}

void logJsonDocument()
{
  serializeJson(jsonDocument, Serial);
  Log.trace(CR);
}

void blinkTick()
{
	blinker.tick();
}

void devicesTick()
{
	devices.tick();
}
