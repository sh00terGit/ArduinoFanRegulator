#include "Arduino.h"
#ifndef FanClass_h
#define FanClass_h



#ifndef RTCDATETIME_STRUCT_H
#define RTCDATETIME_STRUCT_H
struct RTCDateTime
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t dayOfWeek;
    uint32_t unixtime;
};
#endif RTCDATETIME_STRUCT_H

class Fan {
  public:
    Fan(uint8_t fan_pin,uint8_t led_fan_pin);
    Fan(uint8_t fan_pin);
    void fan_tick(RTCDateTime &dt);
    void fan_init();    
    void setEnabled(bool e);
    uint8_t start_min = 0 ;
    uint8_t finish_min = 0;
  private:
    boolean firstStart = true;
    uint8_t fan_pin;
    uint8_t led_fan_pin;
    uint8_t waiting_min;
    uint8_t duration_min;

    bool enabled;
};
#endif FanClass_h
