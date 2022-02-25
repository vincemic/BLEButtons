#include "LEDCommand.h"


LEDCommand::LEDCommand(Devices* devices) {
    name = "led";
    this->devices = devices;

}

void LEDCommand::OnExecute(JsonVariant commandJson) {

}

