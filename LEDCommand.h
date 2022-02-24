#pragma once
#include "Command.h"

class LEDCommand : public Command {

public:
    LEDCommand();
    virtual void OnExecute(JsonVariant commandJson);
 };