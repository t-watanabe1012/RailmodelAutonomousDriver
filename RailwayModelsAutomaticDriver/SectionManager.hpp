#pragma once

#include "Section.hpp"
#include <Arduino.h>

//分周比1として出力を400とすると20kHzのPWMとできる
#define MOTOR_DRV_MAX 400
//出力の最大・最小
#define MOTOR_DRV_OUT_MAX 255
#define MOTOR_DRV_OUT_MIN 30

class SectionManager {
private:
  Section* _sections;
  //区間数を表す
  size_t _section_num;
public:
  template<size_t SIZE>
  SectionManager(Section (&section_array)[SIZE])
    : _sections(section_array),
      _section_num(SIZE) {}
  void init_for_sections();
  void updateSectionState();
};
