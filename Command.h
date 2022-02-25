#pragma once
#include "ArduinoJson.h"

class Command {

public: 
const char* name;

   Command();
   void Execute(const char * commandName, JsonVariant commandJson);
   virtual void OnExecute(JsonVariant commandJson){};
} ;