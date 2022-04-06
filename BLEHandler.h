#pragma once
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoLog.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define BLE_SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define BLE_CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define BLE_CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


class BLEHandlerClass
{
public:
  BLEHandlerClass();
  void begin();
  void tick();
  void update(bool);

private:
  const char *btDeviceName = "BLEButtons";
  BLEServer *pServer = NULL;
  BLECharacteristic *pTxCharacteristic;
  bool connected = false;
  uint8_t txValue = 0;
};

extern BLEHandlerClass BLEHandler;


