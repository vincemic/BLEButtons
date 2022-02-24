#include "Command.h"


Command::Command() {

}

void Command::Execute(const char * commandName, JsonVariant commandJson) {
    if(strcmp(name, commandName )) {
        OnExecute(commandJson);
    }
}