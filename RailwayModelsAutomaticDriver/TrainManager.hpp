#include "Train.hpp"

class TrainManager{
private :
  Train* _trains;
  size_t _train_num;
public : 
  void trainSpdControl();
  template <size_t SIZE>
  TrainManager(Train (&train_array)[SIZE]):
    _trains(train_array),
    _train_num(SIZE)
  {

  }
};