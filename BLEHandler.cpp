#include "BLEHandler.h"
#include <ArduinoLog.h>
#include "BlinkerHandler.h"

void feederTask2(void *parameter)
{

  while (true)
  {
    if (BLEHandler._dREQFlag)
    {
      // Reset the flag right away to allow it to capture further interrupts
      BLEHandler._dREQFlag = false;
      size_t item_size;
      char *message = (char *)xRingbufferReceive(BLEHandler.ringBuffer, &item_size, pdMS_TO_TICKS(1000));

      // Check received item
      if (message != NULL)
      {
        BLEHandler.receivedMessageCallbback(message, item_size);
        vRingbufferReturnItem(BLEHandler.ringBuffer, (void *)message);
      }
 
    }

    vTaskDelay(40);
  }
}

BLEHandlerClass::BLEHandlerClass()
{
}

void BLEHandlerClass::begin(ReceivedMessageCallbback receivedMessageCallbback)
{

  this->receivedMessageCallbback = receivedMessageCallbback;

  // Create the BLE Device
  BLEDevice::init(btDeviceName);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new HandlerServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(BLE_SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
      BLE_CHARACTERISTIC_UUID_TX,
      BLECharacteristic::PROPERTY_NOTIFY);

  pTxCharacteristic->setCallbacks(new HandlerCharacteristicCallbacks());

  // This descriptor is required by BLE spec to allow notifications
  // This is only seems to be checked by Win OS stacks currently
  pTxCharacteristic->addDescriptor(new BLE2902());

  pRxCharacteristic = pService->createCharacteristic(
      BLE_CHARACTERISTIC_UUID_RX,
      BLECharacteristic::PROPERTY_WRITE);

  pRxCharacteristic->setCallbacks(new HandlerCharacteristicCallbacks());

  // Start the service
  pService->start();

  // Create ring buffer

  ringBuffer = xRingbufferCreate(500, RINGBUF_TYPE_NOSPLIT);
  if (ringBuffer == NULL)
  {
    printf("Failed to create ring buffer\n");
  }

  // Now set up two tasks to run independently.
  xTaskCreatePinnedToCore(
      feederTask2, "feederTask2" // A name just for humans
      ,
      4096 // This stack size can be checked & adjusted by reading the Stack Highwater
      ,
      NULL, 2 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
      ,
      NULL, 0);

  setConnected(false);
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

  // Copy string into ring buffer including the null terminator
  xRingbufferSendFromISR(ringBuffer, value.c_str(), value.length() + 1, &woke);
  _dREQFlag = true;
}

void BLEHandlerClass::send(const char *message)
{
  if (connected)
  {
    pTxCharacteristic->setValue(message);
    pTxCharacteristic->notify();
    Log.traceln(F("[BLEHandler] Sent %s"), message);
  }
}

BLEHandlerClass BLEHandler;
