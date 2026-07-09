#pragma once

#include "Sensor.hpp"
#define NO_LED 255

//信号機の状態(点灯している色)を表す
/*RE:停止(赤1個)・YY:警戒(黄色2個)・YE:注意(黄色1個)・YG:減速(黄色1個・緑1個)・GR:進行(緑1個)*/
typedef enum{
  RE, YY, YE, YG, GR
}SignalLightsStatus;

//信号機を表す構造体
class SignalLight{
public:
  SignalLightsStatus status;
  //防護区間
  Section* protect_section;
  uint8_t re_pin;
  uint8_t gr_pin;
  uint8_t ye_pin;
  uint8_t yg_pin;
  uint8_t yy_pin;

  SignalLight(Section& protectSection, uint8_t rePin = NO_LED, uint8_t grPin = NO_LED, uint8_t yePin = NO_LED, uint8_t ygPin = NO_LED, uint8_t yyPin = NO_LED):
  status(GR),
  protect_section(&protectSection),
  re_pin(rePin),
  gr_pin(grPin),
  ye_pin(yePin),
  yg_pin(ygPin),
  yy_pin(yyPin)
  {}
};
