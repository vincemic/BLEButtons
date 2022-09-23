#include "DeviceHandler.h"
#include <TaskScheduler.h>

void deviceButtonCallback(Button &btn, bool isOn);
void InitializeFiles();
void protocolReportCallback();
Task blinkTask;
Task devicesTask;
Task wifiTask;
Task batteryTask;
void blinkTick();
void devicesTick();
void wifiTick();
void batteryTick();
int wifiAttempts = 0;
