#pragma once
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoLog.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define BLE_SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID

typedef void (*ReceivedMessageCallbback)(const char *messge, size_t size);

class BLEHandlerClass
{
  friend class HandlerServerCallbacks;
  friend class HandlerCharacteristicCallbacks;

public:
  BLEHandlerClass();
  void begin(ReceivedMessageCallbback receivedMessageCallbback);
  void tick();
  bool isConnected();
  void send(const char *message);
  const char *deviceName = "BLEButtons";
  volatile boolean _dREQFlag = false;
  ReceivedMessageCallbback receivedMessageCallbback;
  void registerDeviceCharacteristic(const char *UUID, const char *name, BLECharacteristicCallbacks *bleCharacteristicCallbacks,uint32_t properties);

private:
  void setConnected(bool);
  IRAM_ATTR void receive(BLECharacteristic *pCharacteristic);

  uint32_t count = 0;

  BLEServer *pServer = NULL;
  BLEService *pService = NULL;
  bool connected = false;
  uint8_t txValue = 0;
};

extern BLEHandlerClass BLEHandler;

class HandlerServerCallbacks : public BLEServerCallbacks
{

public:
  void onConnect(BLEServer *pServer)
  {
    BLEHandler.setConnected(true);
    Log.infoln(F("[BLEHandler] Device connected"));
  }

  void onDisconnect(BLEServer *pServer)
  {
    Log.infoln(F("[BLEHandler] Device disconnected"));
    BLEHandler.setConnected(false);
  }
};

class HandlerCharacteristicCallbacks : public BLECharacteristicCallbacks
{

public:
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    BLEHandler.receive(pCharacteristic);
  }

};