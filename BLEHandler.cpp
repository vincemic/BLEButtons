#include "BLEHandler.h"
#include <ArduinoLog.h>
#include "BlinkerHandler.h"
#include "DeviceHandler.h"

BLEHandlerClass::BLEHandlerClass()
{
}

void BLEHandlerClass::begin(ReceivedMessageCallbback receivedMessageCallbback)
{
  this->receivedMessageCallbback = receivedMessageCallbback;

  int generalRemote = 384;
  // Create the BLE Device
  BLEDevice::init(deviceName);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new HandlerServerCallbacks());

  // Create the BLE Service - number of handlers are import with large characteristic lists
  pService = pServer->createService(BLEUUID(BLE_SERVICE_UUID), 50);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(BLEUUID((uint16_t) 0x2a01),BLECharacteristic::PROPERTY_READ );
  pCharacteristic->setValue(generalRemote); //Appearance characteristic set for general remote control

  for (Button button : DeviceHandler.buttons)
  {
    registerDeviceCharacteristic((const char *) button.bleIdentifier, (const char *) button.name, new HandlerCharacteristicCallbacks(), BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
  }

  for (Led led : DeviceHandler.leds)
  {
    registerDeviceCharacteristic((const char *) led.bleIdentifier, (const char *)  led.name, new HandlerCharacteristicCallbacks(), BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ);
  }

  setConnected(false);

  // Start the service
  pService->start();
}

void BLEHandlerClass::tick()
{

  if (connected)
  {
    /*
    char message[100];
    count++;
    snprintf_P(message, sizeof(message), PSTR("This is an event: %d"), count);
    pTxCharacteristic->setValue(message);
    pTxCharacteristic->notify();
    Log.traceln(F("[BLEHandler] Sent %s"), message);
    */
  }
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

void BLEHandlerClass::receive(BLECharacteristic *pCharacteristic)
{
  BaseType_t woke;
  std::string value = pCharacteristic->getValue();
}

void BLEHandlerClass::send(const char *message)
{
  if (connected)
  {
    // pTxCharacteristic->setValue(message);
    // pTxCharacteristic->notify();
    Log.traceln(F("[BLEHandler] Sent %s"), message);
  }
}

void BLEHandlerClass::registerDeviceCharacteristic(const char *UUID, const char *name, BLECharacteristicCallbacks *bleCharacteristicCallbacks, uint32_t properties)
{
  int defaultValue = 0;
  Log.traceln(F("[BLEHandler] Registering characteristic %s"), UUID);

  BLEDescriptor *pBLEDescriptor = new BLEDescriptor(BLEUUID("2901"));
  pBLEDescriptor->setValue(name);

  // Create a BLE Characteristic
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(UUID, properties);
  pCharacteristic->setValue(defaultValue);

  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->addDescriptor(pBLEDescriptor);
  pCharacteristic->setCallbacks(bleCharacteristicCallbacks);
}

BLEHandlerClass BLEHandler;
