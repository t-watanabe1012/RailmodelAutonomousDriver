#include "SensorManager.hpp"

void SensorManager::sensorCheck() {
  switch (_state) {
    case IDLE:
      if (current_ms - previous_ms >= CHECK_INTERVAL_MS) {
        previous_ms = current_ms;
        //センサーのLEDを点灯させる//LOWで点灯になる点に注意
        PORTD &= ~(1 << _on_pin);
        _state = ON;
      }
      break;
    case ON:
      if (current_ms - previous_ms >= STABLE_INTERVAL_MS) {
        previous_ms = current_ms;
        //各ピンの状態を記録する->フォトトランジスタ導通時(外光の影響を受けているまたは踏まれたとき)はLOWなので、反転して記録する
        _LED_ON_sensor_val = ~PINA;
        //センサーのLEDを消灯させる
        PORTD |= (1 << _on_pin);
        _state = OFF;
      }
      break;
    case OFF:
      if (current_ms - previous_ms >= STABLE_INTERVAL_MS) {
        previous_ms = current_ms;
        //各ピンの状態を記録する->外光の影響を受けていないときはHIGH
        _LED_OFF_sensor_val = PINA;
        //点灯時の状態と合わせて最終判定とする
        _sensor_sesult = _LED_ON_sensor_val & _LED_OFF_sensor_val;
        //センサーのLEDを消灯させる
        PORTD |= (1 << _on_pin);
        _state = IDLE;
        //センサーが踏まれていると判定した場合、そのセンサのlatest_passed_timeを更新する
        byte mask = 0B00000001;
        for (int i = 0; i < _sensor_num; i++) {
          if (_sensor_sesult & mask) {
            _sensors[i].state = DETECTED;
            _sensors[i].latest_passed_ms = current_ms;
          }
          mask = mask << 1;
        }
      }
      break;
    default:
      break;
  }
  //最後に踏まれた時刻から一定時間経過後にセンサーの通過判定解除
  for (int i = 0; i < _sensor_num; i++) {
    if (current_ms - _sensors[i].latest_passed_ms >= PASSING_INTERVAL_MS) {
      _sensors[i].state = NOT_DETECTED;
    }
  }
}
