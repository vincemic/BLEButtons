#include "BLEHandler.h"
#include <ArduinoLog.h>
#include "BlinkerHandler.h"
#include "DeviceHandler.h"

BLEHandlerClass::BLEHandlerClass()
{
}

void BLEHandlerClass::begin()
{

  int generalRemote = 384;
  // Create the BLE Device
  BLEDevice::init(deviceName);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new HandlerServerCallbacks());

  // Create the BLE Service - number of handlers are import with large characteristic lists
  pService = pServer->createService(BLEUUID(BLE_SERVICE_UUID), 75);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(BLEUUID((uint16_t)0x2a01), BLECharacteristic::PROPERTY_READ);
  pCharacteristic->setValue(generalRemote); // Appearance characteristic set for general remote control

  for (Button &button : DeviceHandler.buttons)
  {
    registerDeviceCharacteristic(&button);
  }

  for (Led &led : DeviceHandler.leds)
  {
    registerDeviceCharacteristic(&led);
  }

  for (Setting &setting : DeviceHandler.settings)
  {
    registerDeviceCharacteristic(&setting);
  }

    for (Player &player : DeviceHandler.players)
  {
    registerDeviceCharacteristic(&player);
  }

  setConnected(false);

  // Start the service
  pService->start();
}

void BLEHandlerClass::setConnected(bool connected)
{
  this->connected = connected;

  if (!connected)
  {
    BlinkerHandler.neoBlue();
    pServer->getAdvertising()->start();
  }
  else
  {
    count = 0;
    BlinkerHandler.neoGreem();
  }
}

bool BLEHandlerClass::isConnected()
{
  return connected;
}


void BLEHandlerClass::registerDeviceCharacteristic(Device *device)
{
  int defaultValue = 0;
  Log.traceln(F("[BLEHandler] Registering characteristic %s"), device->bleIdentifier);

  BLEDescriptor *pNameDescriptor = new BLEDescriptor(BLEUUID("2901"));
  pNameDescriptor->setValue(device->name);

  // Create a BLE Characteristic
  device->pCharacteristic = pService->createCharacteristic(device->bleIdentifier, device->getProperties());

  device->pCharacteristic->setValue(defaultValue);

  device->pCharacteristic->addDescriptor(new BLE2902());
  device->pCharacteristic->addDescriptor(pNameDescriptor);

  device->pCharacteristic->setCallbacks(device);
}

BLEHandlerClass BLEHandler;
