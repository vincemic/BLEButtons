

//This example code is in the Public Domain (or CC0 licensed, at your option.)
//By Evandro Copercini - 2018
//
//This example creates a bridge between Serial and Classical Bluetooth (SPP)
//and also demonstrate that SerialBT have the same functionalities of a normal Serial

#include "BluetoothSerial.h"
#include <ArduinoJson.h>


#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#include "Adafruit_seesaw.h"
#include "LoopbackStream.h"
#include "LEDCommand.h"


#define DEFAULT_I2C_ADDR 0x3A
#define BUFFER_SIZE 500
#define RETURN '\r'
#define LINEFEED '\n'

uint8_t inputCharacter;
uint8_t markerCount = 0;
uint16_t pid;
uint8_t year, mon, day;

struct button {
  uint8_t index = 0;
  uint8_t pin = 0 ;
  uint8_t led = 0 ;
  uint8_t debounce = 0;
  button(uint8_t index, uint8_t pin, uint8_t led) {
    this->index = index;
    this->pin = pin;
    this->led = led;
  }
};
button buttons[] = {{1, 18, 12}, {2, 19, 13}, {3, 20, 0}, {4, 2, 1}};
LoopbackStream protocolStream(BUFFER_SIZE);
DynamicJsonDocument jsonDocument(BUFFER_SIZE);
Adafruit_seesaw ss;
BluetoothSerial SerialBT;
Command commands[] = { LEDCommand() };

void setup() {
  Serial.begin(115200);
  SerialBT.begin(F("BLEButtons")); //Bluetooth device name
  Serial.println(F("The device started, now you can pair it with bluetooth!"));

  if (!ss.begin(DEFAULT_I2C_ADDR)) {
    Serial.println(F("seesaw not found!"));
    while (1) delay(1000);
  }

  ss.getProdDatecode(&pid, &year, &mon, &day);
  Serial.print(F("seesaw found PID: "));
  Serial.print(pid);
  Serial.print(F(" datecode: "));
  Serial.print(2000 + year); Serial.print("/");
  Serial.print(mon); Serial.print("/");
  Serial.println(day);

  if (pid != 5296) {
    Serial.println(F("Wrong seesaw PID"));
    while (1) delay(10);
  }

  Serial.println(F("seesaw started OK!"));

  for ( button btn : buttons) {
    ss.pinMode(btn.pin, INPUT_PULLUP);
    ss.analogWrite(btn.led, 0);
  }
}

void loop() {
  while (SerialBT.available()) {
    process(SerialBT.read());
  }

  bool switchOn = false;

  for ( button &btn : buttons) {

    switchOn = !ss.digitalRead(btn.pin);

    if (switchOn) {
      ss.analogWrite(btn.led, 255);
      if (btn.debounce < 1) {
        log(F("Button pressed"));
        btWriteButtonMessage(btn.index);
        btn.debounce = 1;
      }
    } else if (!switchOn) {
      ss.analogWrite(btn.led, 0);
      if (btn.debounce > 0)
        btn.debounce--;
    }

  }

}

void btWriteMessage(String* message) {
  SerialBT.write(RETURN);
  SerialBT.write(LINEFEED);
  SerialBT.write((const uint8_t*)message->c_str(), message->length());
  SerialBT.write(RETURN);
  SerialBT.write(LINEFEED);
}

void btWriteButtonMessage(int number)
{
  SerialBT.write('b');
  SerialBT.write('0' + number);
  SerialBT.write(RETURN);
  SerialBT.write(LINEFEED);
}

void process(uint8_t character) {

  if (character < 0)
    return;

  protocolStream.write(character);

  if (character == RETURN)
    markerCount++;
  else
    markerCount = 0;

  if (markerCount >= 2) {
    processDocument();
    markerCount = 0;
  }

}

void processDocument() {
  DeserializationError error = deserializeJson(jsonDocument, protocolStream);
  protocolStream.clear();

  if (error) {
    logError(F("deserializeJson() failed: "));
    logError(error.f_str());

    return;
  } else {

    logJsonDocument();
    log(F("message received"));
    
    JsonVariant commandJson = jsonDocument["command"];

    if (commandJson.isNull()) {
      logError(F("command not found"));
      return;
    } else
    {
      log(F("command found"));
      const char * commandName = commandJson.as<const char*>();

      for(Command& command: commands) {
          command.Execute(commandName, commandJson);
      }
      
    }
  }
}

void logError(const __FlashStringHelper* message ) {
  Serial.print(LINEFEED);
  Serial.print(F("Error: "));
  Serial.print(message);
  Serial.print(LINEFEED);
}

void logError(char* message ) {
  Serial.print(F("\nError: "));
  Serial.print(message);
  Serial.print(LINEFEED);
}

void log(const __FlashStringHelper* message) {
  Serial.print(message);
  Serial.print(LINEFEED);
}

void log(char* message) {
  Serial.print(message);
  Serial.print(LINEFEED);
}

void logJsonDocument() {
  serializeJson(jsonDocument, Serial);
  Serial.print(LINEFEED);
}

