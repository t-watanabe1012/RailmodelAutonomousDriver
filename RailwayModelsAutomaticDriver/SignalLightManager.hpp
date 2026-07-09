#pragma once

#include <Arduino.h>
#include "Section.hpp"
#include "SignalLight.hpp"

class SignalLightManager {
private:
  SignalLight* _signalLights;
  //信号機の個数
  size_t _signalLights_num;
public:
  template<size_t SIZE>
  SignalLightManager(SignalLight (&signalLights_array)[SIZE]):
    _signalLights (signalLights_array),
    _signalLights_num(SIZE)
   {}
  void signalLights_update();
  void init_for_signalLightManager();
};
