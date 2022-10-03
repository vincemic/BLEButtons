/*********************************************************************************
 * data.h - holds data structures for WebTachometer.ino
 * 
 * -- Yuri - Aug 2021
*********************************************************************************/

#ifndef TACH_H
#define TACH_H

struct Tach {
  double   rpm = 0;
  uint32_t last_triggered = 0;
  uint8_t  num_poles = 1;
};


#endif // DATA_H
