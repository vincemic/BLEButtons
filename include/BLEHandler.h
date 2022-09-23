#pragma once
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoLog.h>
#include "DeviceHandler.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define BLE_SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID

class BLEHandlerClass
{
  friend class HandlerServerCallbacks;
  friend class HandlerCharacteristicCallbacks;

public:
  BLEHandlerClass();
  void begin();
  bool isConnected();
  const char *deviceName = "BLEButtons";
  volatile boolean _dREQFlag = false;
  void registerDeviceCharacteristic(Device *device);

private:
  void setConnected(bool);

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
