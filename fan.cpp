#include "fan.h"

Fan::Fan(uint8_t fan_pin,uint8_t led_fan_pin){
  this->fan_pin = fan_pin;
  this->led_fan_pin = led_fan_pin;
  pinMode(this->fan_pin,OUTPUT);
  pinMode(this->led_fan_pin, OUTPUT);
  this->waiting_min = 2;
  this->duration_min = 1;
  
}
Fan::Fan(uint8_t fan_pin){
  this->fan_pin = fan_pin;
  pinMode(this->fan_pin,OUTPUT);
}
void Fan::fan_init(){
  pinMode(this->fan_pin,OUTPUT);
  pinMode(this->led_fan_pin, OUTPUT);
  this->setEnabled(false);
  Serial.println("Init FAN");
}




void Fan::setEnabled(bool e) {
  this->enabled = e;
  digitalWrite(this->fan_pin, e ? HIGH : LOW);
  digitalWrite(this->led_fan_pin, e ? HIGH : LOW);
}

void Fan::fan_tick(RTCDateTime &dt) {
  
  if(this->firstStart) { 
    this->firstStart = false;
    this->start_min = dt.minute;    
    this->finish_min = this->start_min + duration_min;
    Serial.println("fan_tick_first");
    this->finish_min  = this->finish_min >= 60 ? this->finish_min-60 :  this->finish_min; 
    Serial.println(this->start_min);
    Serial.println(this->finish_min);
  }  
  if (dt.minute ==  this->start_min ) {
    this->setEnabled(true);
    
    Serial.print("next finish time = ");
    Serial.println(this->finish_min);
    this->start_min = this->finish_min+waiting_min;        
    this->start_min  = (this->start_min >= 60) ? this->start_min- 60 :  this->start_min;       
  }
  if (dt.minute ==  this->finish_min) {
    this->setEnabled(false);
    Serial.print("next start time  = ");
    Serial.println(start_min);
    this->finish_min = this->start_min + duration_min;
    this->finish_min  = this->finish_min >= 60 ? this->finish_min-60 :  this->finish_min;  
  }
 
}
