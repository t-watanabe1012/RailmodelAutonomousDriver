#pragma once

#include <Arduino.h>
class Section;

extern unsigned long current_ms;
//列車を表す構造体
class Train{
private : 
public : 
  //目標スピード
  uint16_t target_speed;
  //現在スピード
  uint16_t current_speed;
  //コンストラクタ
  Train():target_speed(0),current_speed(0){};
  void putTrain(Section& section);
};
