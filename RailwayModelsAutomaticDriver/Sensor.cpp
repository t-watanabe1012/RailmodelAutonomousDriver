#include "Sensor.hpp"

Sensor::Sensor(byte pin)
  :
    pin(pin),
    state(NOT_DETECTED),
    latest_passed_ms(0)
{}
