#include "SignalLightManager.hpp"

/*
現状、個別に点灯させているが、ダイナミック点灯にしたい
現状では((信号機の台数)*(信号機の灯数))本のピンが必要になるが、ピン数削減につながる。

接続案
5灯式まで採用の場合:
赤,黄1,緑,黄2,黄3それぞれのアノードで1ピンずつ5ピン使う
各信号機に1ピンカソードをまとめてつなぐ
これにより、((信号機の台数)+(信号機の最大灯数))本のピンで制御でき、複数の信号機を実装するならこのほうが有利。

点灯させたいタイミングでアノード/カソードをうまくつなぐ
配線数は減るがコードが複雑になる。レジスタで操作するのがほぼ必須。
処理が重くなってくると各色のLEDの明るさに違いが出るので、その点は要注意。
1000Hzくらいなら人間の目には連続点灯に見えるか。
*/
void SignalLightManager::init_for_signalLightManager(){
  for(int i = 0; i < _signalLights_num; i ++){
  	//カソードピンを出力に設定
  	DDRC |= (1 << i);
  	//カソードピンは点灯しないように設定しておく
  	PORTC|= (1 << i);
  }
  //赤・黄・青のピンを出力に設定して消灯しておく
  DDRB |=  0b00000111;
  PORTB&= ~0b00000111;
}

void SignalLightManager::signalLights_update(){
  static uint8_t count = 0;
  //各信号の状態を確認し、マスクに入れておく
  uint8_t red_mask = 0;
  uint8_t yel_mask = 0;
  uint8_t grn_mask = 0;
  SectionState state;
  for(int i = 0; i < _signalLights_num; i ++){
    state = _signalLights[i].protect_section->is_occupied;
    if(state == SECTION_EMPTY || state == SECTION_PREPARE){
      //この区間に列車がいない場合、一つ先の区間を見ておく
      state = _signalLights[i].protect_section->next_section->is_occupied;
      if(state == SECTION_EMPTY || state == SECTION_PREPARE){
      	//一つ先の区間にも列車がいないとき:緑信号
        _signalLights[i].status = GR;
      	grn_mask |= (1 << i);
      }else{
      	//一つ先の区間に列車がいるとき:黄信号
        _signalLights[i].status = YE;
      	yel_mask |= (1 << i);
      }
    }else{
      //この区間に列車がいるとき:赤信号
      _signalLights[i].status = RE;
      red_mask |= (1 << i);
    }
  }
  PORTB = 0;
  //点灯する色のカソードのマスクをもとに出力を設定
  if(count == 0){PORTC = ~red_mask;}
  if(count == 1){PORTC = ~yel_mask;}
  if(count == 2){PORTC = ~grn_mask;}
  //点灯する色のアノードのマスクを設定
  uint8_t anode_mask = 1 << count;
  PORTB = anode_mask;
  count ++;
  if(count > 3)count = 0;
}
