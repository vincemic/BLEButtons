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

#define VS1053_RESET   -1     // VS1053 reset pin (not used!)
#define VS1053_CS 32   // VS1053 chip select pin (output)
#define VS1053_DCS 33  // VS1053 Data/command select pin (output)
#define CARDCS 14      // Card chip select pin
#define VS1053_DREQ 15 // VS1053 Data request, ideally an Interrupt pin

// Scheduler
Scheduler scheduler;
Task blinkTask;
Task devicesTask;
Task protocolTask;
Task wifiTask;
Task batteryTask;

Blinker blinker;
Devices devices(&deviceButtonCallback);
ProtocolProcessor protocolProcessor(&protocolLedCallback, &protocolReportCallback);
ESPBattery battery(&batteryCallback);
SoundPlayer soundPlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);

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
 
  if (! soundPlayer.begin()) { 
     Log.errorln(F("Couldn't find VS1053"));
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

  //soundPlayer.sciWrite(VS1053_REG_WRAMADDR, VS1053_GPIO_DDR);
  //soundPlayer.sciWrite(VS1053_REG_WRAM, 3);
  //soundPlayer.sciWrite(VS1053_REG_WRAMADDR, VS1053_GPIO_ODATA);
  //soundPlayer.sciWrite(VS1053_REG_WRAM, 0);
  //delay(100);
  //soundPlayer.softReset();
  
  soundPlayer.setVolume(0,0);  
  soundPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int

  if (!SD.begin(CARDCS)) {
    Log.errorln(F("SD failed, or not present"));
  } else {
    // list files
    printDirectory(SD.open("/"), 0);
  }
  

  
  soundPlayer.playFullFile("/hello001.mp3"); 
}

void loop()
{
  scheduler.execute();

  /* eziya76, when IRQ fires, handler set DREQFlag */
  if(soundPlayer.DREQFlag) {
    /* feed buffer when DREQ interrupt */
    soundPlayer.feedBuffer();
    soundPlayer.DREQFlag = false;
  }
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

void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       //Serial.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Log.trace("\t");
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Log.traceln("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Log.trace("\t\t");
       Log.traceln("%d",entry.size());
     }
     entry.close();
   }
}