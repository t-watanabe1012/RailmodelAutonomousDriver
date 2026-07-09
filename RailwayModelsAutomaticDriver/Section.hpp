#pragma once

#include "Train.hpp"
#include "Sensor.hpp"

//区間の状態を表す
typedef enum {
  SECTION_EMPTY = 0,         //在線なし
  SECTION_PREPARE = 1,       //在線なし(受け入れ準備)
  SECTION_ENTERING = 2,      //在線あり(入りかかっている)
  SECTION_ENTERED = 3,       //在線あり(完全に入っている)
  SECTION_EXITING = 4,       //在線あり(出かかっている)
  SECTION_DISCONNECTED = 5,  //分岐器で別方向が開通している場合
} SectionState;

//各区間を表す構造体
/*区間と信号機・センサーの対応を紐付けておく*/
class Section {
public:
  //この区間に列車が在線(出口未到着)・在線(出口到着中)・非在線
  SectionState is_occupied;
  //前の区間
  Section* prev_section;
  //次の区間
  Section* next_section;
  //入口センサー:この区間の入口センサ
  Sensor* entrance_sensor;
  //出口センサー:次の区間の入口センサ
  Sensor* exit_sensor;
  //モータードライバの前進ピンの設定
  volatile uint16_t* fwd_pin;
  //モータードライバの後退ピンの設定
  volatile uint16_t* rev_pin;
  //列車がいる場合、そのアドレスを持っておく
  Train* train;

  Section(Section& prevSection, Section& nextSection, 
    Sensor& entranceSensor, Sensor& exitSensor, 
    volatile uint16_t& fwdPin, volatile uint16_t& revPin)
  :
  is_occupied(SECTION_EMPTY),
  prev_section(&prevSection),
  next_section(&nextSection),
  entrance_sensor(&entranceSensor),
  exit_sensor(&exitSensor),
  fwd_pin(&fwdPin),
  rev_pin(&revPin),
  train(nullptr)
  {}
};
