#pragma once

#include <Arduino.h>
#include "Sensor.hpp"

//チェック間隔(ms)秒単位
#define CHECK_INTERVAL_MS 8UL
//LEDの点灯・消灯から計測まで待つ時間(ms単位)
#define STABLE_INTERVAL_MS 1UL
//センサーが一度検出した後、下記の時間は列車が直上を通過中とみなす
#define PASSING_INTERVAL_MS 500

typedef enum {
  IDLE = 0,
  ON = 1,
  OFF = 2
} SensorManagerState;

class SensorManager {
private:
  Sensor* _sensors;
  //センサーの個数を表す
  size_t _sensor_num;
  int _on_pin;
  SensorManagerState _state;
  //LEDがONのときにセンサーがHIGHかLOWかを記録
  uint8_t _LED_ON_sensor_val;
  //LEDがOFFのときにセンサーがHIGHかLOWかを記録
  uint8_t _LED_OFF_sensor_val;
  //各センサーの判定結果を記録
  uint8_t _sensor_sesult;
public:
  template<size_t SIZE>
  SensorManager(Sensor (&sensor_array)[SIZE])
    : _sensors(sensor_array),
      _sensor_num(SIZE) {
    //38番で一括制御
    _on_pin = PD7;
    _state = IDLE;
    for (int i = 0; i < _sensor_num; i++) {
      //センサーの信号線はPORTAに接続する予定なので、入力にし、内部プルアップを有効とする
      DDRA &= ~(1 << _sensors[i].pin);
      PORTA |= (1 << _sensors[i].pin);
    }
    //センサーの電源を一括OFFにして待機する
    DDRD |= (1 << _on_pin);
    PORTD &= ~(1 << _on_pin);
  }
  void sensorCheck();
};

extern unsigned long previous_ms;
extern unsigned long current_ms;
