/*********************************************************************************
 * data.h - holds data structures for WebTachometer.ino
 * 
 * -- Yuri - Aug 2021
*********************************************************************************/

#ifndef DATA_H
#define DATA_H

struct Tach {
  uint8_t  pin;
  double   rpm = 0;
  uint32_t last_triggered = 0;
  uint8_t  num_poles = 1;
};

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


#endif // DATA_H
