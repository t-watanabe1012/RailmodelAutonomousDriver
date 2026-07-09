#pragma once
#include <Arduino.h>

//センサーの状態を表す:非検知, 検知
typedef enum {
  NOT_DETECTED, DETECTED
}SensorState;

class Sensor{
  private:
    
  public:
    byte pin;
    SensorState state;
    unsigned long latest_passed_ms;
    Sensor(byte);
};
