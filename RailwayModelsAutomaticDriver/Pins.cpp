#include "Pins.h"

DigitalPins::DigitalPins(uint8_t pinNo)
  :
  _port(digitalPinToPort(pinNo)),
  _bit_mask(digitalPinToBitMask(pinNo))
{}