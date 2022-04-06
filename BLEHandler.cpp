#include "BLEHandler.h"
#include <ArduinoLog.h>
#include "BlinkerHandler.h"

BLEHandlerClass::BLEHandlerClass()
{
  
}

void BLEHandlerClass::begin()
{
  
  // Create the BLE Device
  BLEDevice::init(btDeviceName);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new HandlerServerCallbacks() );

  // Create the BLE Service
  BLEService *pService = pServer->createService(BLE_SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
      BLE_CHARACTERISTIC_UUID_TX,
      BLECharacteristic::PROPERTY_NOTIFY);

  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
      BLE_CHARACTERISTIC_UUID_RX,
      BLECharacteristic::PROPERTY_WRITE);

  pRxCharacteristic->setCallbacks(new HandlerCharacteristicCallbacks());

  // Start the service
  pService->start();

  update(false);

}

void BLEHandlerClass::tick()
{
}


void BLEHandlerClass::update(bool connected) {
  this->connected = connected;
  
  if(!connected) {
    BlinkerHandler.neoBlue();
    pServer->getAdvertising()->start();
  } else {
    BlinkerHandler.neoGreem();
  }
}

BLEHandlerClass BLEHandler;

class HandlerServerCallbacks : public BLEServerCallbacks
{

public:
  void onConnect(BLEServer *pServer)
  {
    BLEHandler.update(true);
    Log.infoln(F("[BLEHandler] Device connected"));
  }

  void onDisconnect(BLEServer *pServer)
  {
    Log.infoln(F("[BLEHandler] Device disconnected"));
    delay(500);
    BLEHandler.update(false);
  }

};

class HandlerCharacteristicCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string rxValue = pCharacteristic->getValue();
    Log.infoln(F("[BLEHandler] received value $s"), rxValue);
  }
};