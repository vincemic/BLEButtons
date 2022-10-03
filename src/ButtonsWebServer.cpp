#include "ButtonsWebServer.h"
#include "tach.h"
#include <SD.h>
#include "ArduinoLog.h"
#include <AsyncElegantOTA.h>

Tach leftTach;
Tach rightTach;

void IRAM_ATTR leftTachTrigger() {
    uint32_t time_now = micros();
    leftTach.rpm =  60000000.0  / (double) (time_now - leftTach.last_triggered) /  (double) leftTach.num_poles;
    leftTach.last_triggered = time_now;
}

void IRAM_ATTR rightTachTrigger() {
    uint32_t time_now = micros();
    rightTach.rpm =  60000000.0  / (double) (time_now - rightTach.last_triggered) /  (double) rightTach.num_poles;
    rightTach.last_triggered = time_now;
}

ButtonsWebServer::ButtonsWebServer(int port):AsyncWebServer(port) {}

void ButtonsWebServer::begin() {

  Log.noticeln(F("Configuring Web Tachometer"));

  leftTach.num_poles = DEFAULT_POLES;
  rightTach.num_poles = DEFAULT_POLES;

  pinMode(LEFTTACH_PIN, INPUT_PULLUP);
  attachInterrupt(LEFTTACH_PIN, leftTachTrigger, FALLING);
  pinMode(RIGHTTACH_PIN, INPUT_PULLUP);
  attachInterrupt(RIGHTTACH_PIN, rightTachTrigger, FALLING);

  Serial.print(F("    Adding web server routes..."));

  this->onNotFound([](AsyncWebServerRequest *request){
    Log.notice(F("Bad request, sending 404..."));
    request->send(404, "text/plain", F("404...that's a hard no, super chief."));
    Log.noticeln(F("SENT"));
  });
  this->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    Log.notice(F("Index page requested..."));
    request->send(SD, "/index.html");
    Log.noticeln(F("SENT"));
  });
  this->on("/favicon.svg", HTTP_GET, [](AsyncWebServerRequest *request){
    Log.notice(F("Favicon requested..."));
    request->send(SD, "/favicon.svg");
    Log.noticeln(F("SENT"));
  });
  this->on("/rpm", HTTP_GET, [](AsyncWebServerRequest *request){
    Log.notice(F("RPM requested..."));
    char buf[32];
    char buf_rpm2[8];
    dtostrf(leftTach.rpm, 3, 1, buf);
    dtostrf(rightTach.rpm, 3, 1, buf_rpm2);
    strcat(buf, ",");
    strcat(buf, buf_rpm2);
    request->send_P(200, "text/plain", buf);
    Log.noticeln(buf);
  });
  this->on("/num_poles", HTTP_GET, [](AsyncWebServerRequest *request){
    Log.notice(F("num_poles requested..."));
    char buf[32];
    char buf_num_poles2[8];
    dtostrf(leftTach.num_poles, 1, 0, buf);
    dtostrf(rightTach.num_poles, 1, 0, buf_num_poles2);
    strcat(buf, ",");
    strcat(buf, buf_num_poles2);
    request->send_P(200, "text/plain", buf);
    Log.noticeln(buf);
  });
  this->on("/num_poles", HTTP_POST, [](AsyncWebServerRequest *request){
    Log.notice(F("num_poles POST received..."));
    AsyncWebParameter* p;
    uint8_t val;
    if (request->hasParam("num_poles1", true)) {
      p = request->getParam("num_poles1", true);
      val = atoi(p->value().c_str());
      leftTach.num_poles = val;
      Log.noticeln(F("  tach1.numpoles set: %d\n"), val);
    }
    if (request->hasParam("num_poles2", true)) {
      p = request->getParam("num_poles2", true);
      val = atoi(p->value().c_str());
      rightTach.num_poles = val;
      Log.noticeln(F("  tach2.numpoles set: %d\n"), val);
    }
    request->send(200);
  });

#ifdef _DEBUG_MODE
  // this allows requests from locally stored documents on the client device
  //   (i.e., C:\users\yuri\Documents\debug.html, /home/yuri/Documents/test.html, etc)
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
#endif

  Log.noticeln(F("DONE"));
  Log.notice(F("    Starting web server..."));
  Log.noticeln(F("READY"));

  AsyncElegantOTA.begin(this); 
  AsyncWebServer::begin();
}