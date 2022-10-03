#ifndef BUTTONSWEBSERVER_H
#define BUTTONSWEBSERVER_H
#include <ESPAsyncWebServer.h>

#define LEFTTACH_PIN 17
#define RIGHTTACH_PIN 18
#define DEFAULT_POLES 4

class ButtonsWebServer : public AsyncWebServer{
  public:
    ButtonsWebServer(int port);
    void begin();
};



#endif 
