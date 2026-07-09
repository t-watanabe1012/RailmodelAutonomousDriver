#pragma once

#include <stdint.h>
#include <Arduino.h>

class DigitalPins{
  private : 
    uint8_t* _port;
    uint8_t _bit_mask;

  public : 
    //コンストラクタ
    DigitalPins(uint8_t);
    //
    void setMode(uint8_t);
};