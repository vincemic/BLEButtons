#pragma once

#include "Command.h"
#include "Devices.h"

class LEDCommand : public Command {

private:
Devices* devices;

public:
    LEDCommand(Devices* devices);
    virtual void OnExecute(JsonVariant commandJson);
 };