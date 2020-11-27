/*
 -переключение между режимами двойной btn_h 
 - в режиме часы выводит актуальное время
 - в режиме секундомер 
  для tm1637  выводит секунды времени) , 
  для lcd :
 -- старт btn_m одинарный  
 -- пауза btn_m двойной
 -- стоп  btn_m  тройной

 Использую:
  https://github.com/AlexGyver/GyverLibs/blob/master/GyverTM1637                  экран 4 digit
  https://github.com/ssilver2007/LCD_1602_RUS                                     экран lcd
  https://github.com/jarzebski/Arduino-DS3231/blob/master/DS3231.h                часы
  https://github.com/AlexGyver/GyverLibs/blob/master/GyverButton/GyverButton.h    кнопки
  https://github.com/AlexGyver/GyverLibs/tree/master/microDS18B20                 датчик температуры

*/

#define DEBUG_ENABLE 
#ifdef DEBUG_ENABLE
#define DEBUG(x) Serial.println(x)
#else
#define DEBUG(x)
#endif

#define _LCD_TYPE 2 // lcd type 2 - 10 контактный 
#define IS_DS18B20 1 // с датчиком темп или без
#define IS_LCD 1   // 0 - digit, 1 lcd
#define LCD_TIME_LINE   0
#define LCD_TIME_HOUR_CELL 1
#define LCD_TIME_MIN_CELL 4
#define LCD_TIME_SEC_CELL 7
#define LCD_TIME_POINT_CELL 3
#define LCD_TEMP_CELL 12
#define SENSOR_PIN 22   // пин для термометра

#define REG_TIME 1
#define REG_SEC 3
#define REG_FAN 2
#define MAX_REG 2

#include "GyverTM1637.h"
#include <Wire.h>
#include "DS3231.h"
#include "GyverButton.h"
#include <LCD_1602_RUS_ALL.h>
#include <microDS18B20.h>
#include "fan.h"

#define FAN_PIN 25
#define LED_FAN_PIN 24

GyverTM1637 disp(14, 15);
DS3231 clock;  // часы
RTCDateTime dt , dt_prev, dt_sek;  //datetime
LCD_1602_RUS <LiquidCrystal> lcd(8, 9, 4, 5, 6, 7 );//For LCD Keypad Shield
GButton btn_m;
GButton btn_h;
MicroDS18B20 sensor(SENSOR_PIN);
Fan fan(FAN_PIN,LED_FAN_PIN);

uint8_t regime, brit = 0;
bool fl_sec_start = false , fl_sec_stop = true, fl_first_pusk = true;
uint32_t  u_delta_sec; // для секундомера delta unixtime

void setup() {
  #ifdef DEBUG_ENABLE
  Serial.begin(9600);
  #endif
  
  regime = REG_TIME;  
  clock.begin();
  //Добавил для теста
  firstStart();
  lcd.begin(16, 2);
  lcd.clear();
  sensor.setResolution(9);
  fan.fan_init();
}

void loop() {
  dt = clock.getDateTime();
  btns_check();
  fan.fan_tick(dt);
  displayWatch();
  
}

void firstStart() {
    clock.setDateTime(__DATE__, __TIME__);
  //clock.setDateTime(2020,11, 6, 8, 58,55);
}

void displayWatch() {
  if (dt.second != dt_prev.second) {
    dt_prev = dt;
    if (regime  == REG_TIME)  displayTime();
    if (regime  == REG_SEC)   displaySec();
    if (IS_DS18B20  == 1 ) displayTemp();
    displayFan();
  } 
}

void displayTime() {                                                     // режим времени
  
    
    IS_LCD == 0 ? disp.displayClock(dt.hour, dt.minute) : lcd_displayClock(dt.hour, dt.minute, dt.second);
    if (dt.second % 2 == 0) {
      IS_LCD == 0 ? disp.point(true) : lcd_displayPoint(true);
    } else {
      IS_LCD == 0 ? disp.point(false) : lcd_displayPoint(false);
    }
  
}

void displaySec() {                                                     //режим секунд
  if (IS_LCD) {
    lcd_secundomer();
  } else {
    disp.point(false);
    disp.displayInt(dt.second);
  }

}


void displayTemp() { // выводим температуру
  int t;
  sensor.requestTemp();
   DEBUG(sensor.getTemp());
  lcd_displayTemp(sensor.getTemp());
}

void displayFan() {
  String str ;
  lcd.setCursor(0, 1);        
  str = "en = "+ castByteToTime(fan.start_min) + " dis = "+castByteToTime(fan.finish_min); 
  lcd.print(str);
}


void btns_check() {
  btns_tick(IS_LCD);
  uint8_t  m = dt.minute;
  uint8_t  h = dt.hour;
  if (regime == REG_TIME and (btn_m.isSingle() or btn_m.isStep())) {    // меняем минуты
    m == 59 ? m = 0 : m++;
    clock.setDateTime(dt.year, dt.month, dt.day, dt.hour, m, 0);
  }

  if (regime == REG_TIME and (btn_h.isSingle() or btn_h.isStep())) {    // меняем часы
    h == 23 ? h = 0 : h++;
    clock.setDateTime(dt.year, dt.month, dt.day, h, dt.minute, dt.second);
  }
  if (btn_h.isDouble()) {                                               // меняем режим
    IS_LCD == 0 ? disp.clear() : lcd.clear();
    regime = (regime+1) >MAX_REG ? 1 : regime+1;
    switch (regime) {
      case REG_SEC :
        lcd.setCursor(0, 1);
        lcd.print("секундомер");
        break;  
      case REG_TIME:
        break;
      case REG_FAN:
        break;
      default:
        regime =  REG_TIME;
        break; 
    }
  }
  if (regime == REG_SEC and btn_m.isDouble()) {                         
    if (IS_LCD) {                                                 // пауза секундомера
     fl_sec_start = true;
    fl_sec_stop = true; 
    } else {                                                      //меняем яркость (только для 4 digit)
      brit == 3 ? brit = 0 : brit++;
      disp.brightness(brit) ;
    }
  }
  if (regime == REG_SEC and btn_m.isSingle()) {     // старт таймера
    fl_sec_start = true ;  
    fl_sec_stop = false;
    dt_sek = dt;         // запоминаем datetime на старте     
  }

 if (regime == REG_SEC and btn_m.isTriple()) {     // финиш таймера
  fl_sec_start = false ;  
  fl_sec_stop = true;
 }  
  
}
  void btns_tick(bool is_lcd) {
    if (!is_lcd) {
      btn_h.tick();
      btn_m.tick();
    } else {
      int analog = analogRead(A0);
      btn_h.tick(analog < 600 && analog > 400);  // left
      btn_m.tick(analog < 100);  // right
    }
  }

  void lcd_displayClock (uint8_t h, uint8_t m, uint8_t s) { // вывод на lcd времени
    String h_s, m_s, s_s;
    h_s = castByteToTime(h);
    m_s = castByteToTime(m);
    s_s = castByteToTime(s);
    DEBUG(h_s + " " + m_s + " " + s_s);
    lcd.setCursor(LCD_TIME_HOUR_CELL, LCD_TIME_LINE); // установка на ячейку часов
    lcd.print(h_s);

    lcd.setCursor(LCD_TIME_MIN_CELL, LCD_TIME_LINE); // установка на ячейку минут
    lcd.print(m_s);

    lcd.setCursor(LCD_TIME_SEC_CELL, LCD_TIME_LINE); // установка на ячейку минут
    lcd.print(s_s);

  }

 void lcd_displayTemp(int t) {  // вывод на lcd температуры
   String temp;
   temp = (t >0) ? "+"+String(t)+"C" : String(t)+"C"; 
   lcd.setCursor(LCD_TEMP_CELL, LCD_TIME_LINE); // установка на ячейку часов
   lcd.print(temp);
 }

  
  void lcd_displayPoint(bool p) {
    lcd.setCursor(LCD_TIME_POINT_CELL, LCD_TIME_LINE); // установка на ячейку точки
    p == true ? lcd.print(":") : lcd.print(" ");
  }

  void lcd_secundomer() {
    uint8_t h, m, s;
    if (!fl_sec_start && fl_sec_stop) { // не начался старт      
      h = 0; m = 0; s = 0;
      lcd_displayClock(h, m, s);     
    }
    if (fl_sec_start && !fl_sec_stop) { //   старт начался
      u_delta_sec = dt.unixtime - dt_sek.unixtime;
      sek2time(u_delta_sec);
     
    }
    if (fl_sec_start && fl_sec_stop) { // пауза
      sek2time(u_delta_sec);
    }
  }

  void sek2time(uint32_t s) {
    uint8_t t[3]; 
    t[0] = s/3600; // часы
    t[1] = s/60; // минуты
    t[2] = s % 60 ; // секунды
    lcd_displayClock(t[0], t[1], t[2]);  
  }

  String castByteToTime(uint8_t t) {
    String res ;
    res = (t < 10 ? "0" + String(t) : String(t));
    return res;
  }
