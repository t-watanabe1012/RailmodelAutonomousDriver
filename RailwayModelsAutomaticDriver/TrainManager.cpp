#include "TrainManager.hpp"

void TrainManager::trainSpdControl(){
  static unsigned long previous_acc_ms = 0;
  static unsigned long previous_blk_ms = 0;
  //10msごとに加速
  if(current_ms - previous_acc_ms >= 10){
    previous_acc_ms = current_ms;
    for(int i = 0; i < _train_num; i ++){
      if(_trains[i].target_speed > _trains[i].current_speed){
          _trains[i].current_speed++;
      }
    }
  }
  //10msごとに減速
  if(current_ms - previous_blk_ms >= 5){
    previous_blk_ms = current_ms;
    for(int i = 0; i < _train_num; i ++){
      if(_trains[i].target_speed < _trains[i].current_speed){
        _trains[i].current_speed--;
      }
    }
  }
}

