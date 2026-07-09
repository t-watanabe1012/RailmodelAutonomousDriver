#include "SectionManager.hpp"

void SectionManager::init_for_sections(){
  /*セクションごとに異なるタイマーを使うので、同期操作が必須。
  同期の手順
  タイマー停止->レジスタ書き込み->カウンタを0にリセット->タイマー再開
  */
  //タイマーの設定を変える前にタイマーを止める
  GTCCR = (1 << TSM);
  /*タイマー1, 3～5の各ピンをphase collect PWMで動作させる
    分周比は8, TOPはICRnとする。 
  */
  //タイマー1のピンを設定する. タイマー1のピンはPB5=11, PB6=12, PB7=13
  //13ピンはOCR0Aに紐付いている可能性があるので、ここで外す。
  TCCR0A &= 0B00111111;
  TCCR1A  = 0B11111100;
  TCCR1B  = 0B00010001;
    DDRB |= (1 << PB5) | (1 << PB6) | (1 << PB7); 
    ICR1  = MOTOR_DRV_MAX;
  //念のためOCR1A~Cをすべて0にしておく
  OCR1A = 0;
  OCR1B = 0;
  OCR1C = 0;
  //タイマー3の設定
  TCCR3A = 0B11111100;
  TCCR3B = 0B00010001;
  //タイマー3のピンはE3～E5(PE3=5, PE4=2, PE5=3)なので、これらを出力に設定する
  DDRE |= (1 << PE3) | (1 << PE4) | (1 << PE5); 
  ICR3 = MOTOR_DRV_MAX;
  //念のためOCR3A~Cをすべて0にしておく
  OCR3A = 0;
  OCR3B = 0;
  OCR3C = 0;
  //タイマー4の設定
  TCCR4A = 0B11111100;
  TCCR4B = 0B00010001;
  //タイマー4のピンはH3～H5(PH3=6, PH4=7, PH5=8)なので、これらを出力に設定する
  DDRH |= (1 << PH3) | (1 << PH4) | (1 << PH5); 
  ICR4 = MOTOR_DRV_MAX;
  //念のためOCR4A~Cをすべて0にしておく
  OCR4A = 0;
  OCR4B = 0;
  OCR4C = 0;
  //タイマー5の設定
  TCCR5A = 0B11111100;
  TCCR5B = 0B00010001;
  //タイマー5のピンはL3～L5(PL3=46, PL4=45, PL5=44)なので、これらを出力に設定する
  DDRL |= (1 << PL3) | (1 << PL4) | (1 << PL5); 
  ICR5 = MOTOR_DRV_MAX;
  //念のためOCR5A~Cをすべて0にしておく
  OCR5A = 0;
  OCR5B = 0;
  OCR5C = 0;
  //カウンタリセット
  TCNT1 = 0;
  TCNT3 = 0;
  TCNT4 = 0;
  TCNT5 = 0;
  //プリスケーラーもリセット(TSM:タイマー同期)
  GTCCR = (1 << TSM) | (1 << PSRSYNC);
  //タイマーを再開
  GTCCR = 0B00000000;
}

void SectionManager::updateSectionState(){
  for(int i = 0; i < _section_num; i ++){
    //入口センサが踏まれたらこの区間に列車が入ってきていることになる
    if(_sections[i].entrance_sensor->state == DETECTED){
      _sections[i].is_occupied = SECTION_ENTERING;
    }
    //入ってきている状態で入口センサを列車が完全に通過した場合
    if(_sections[i].is_occupied == SECTION_ENTERING && _sections[i].entrance_sensor->state == NOT_DETECTED){
      _sections[i].is_occupied = SECTION_ENTERED;
    }
    //出口センサが踏まれたらこの区間から列車が出て行っていることになる
    if(_sections[i].exit_sensor->state == DETECTED){
      _sections[i].is_occupied = SECTION_EXITING;
    }
    //出て行っている状態で出口センサが踏まれていなければこの区間から完全に出て行ったことになる
    if(_sections[i].is_occupied == SECTION_EXITING && _sections[i].exit_sensor->state == NOT_DETECTED){
      _sections[i].is_occupied = SECTION_EMPTY;
      //列車がいなくなったのでtrainをNULLにする
      _sections[i].train = NULL;
    }
    //この区間に列車がなく、前の区間に列車がいる場合
    if(_sections[i].is_occupied == SECTION_EMPTY && _sections[i].prev_section->is_occupied == SECTION_ENTERED){
      _sections[i].is_occupied = SECTION_PREPARE;
      //いなくなったのでtrainをNULLにする
      _sections[i].train = _sections[i].prev_section->train;
    }

    //列車の最高速度更新
    int val = analogRead(A0)/4;
    /*完全に入ったら処理*/
    if(_sections[i].is_occupied == SECTION_ENTERED){
      //前に列車がいるかどうかで最高速度を変える
      switch(_sections[i].next_section->is_occupied){
        //前に列車がない
        case SECTION_EMPTY : 
        case SECTION_PREPARE : 
          if(_sections[i].train != NULL){
            //OCRxnの範囲は0~400だが、280で最大出力を70%程度にしておく。
            _sections[i].train->target_speed = min(val, MOTOR_DRV_OUT_MAX);
          }
          break;
        //前に列車がある
        case SECTION_ENTERING :
        case SECTION_ENTERED :
        case SECTION_EXITING :
        default : 
          if(_sections[i].train != NULL){
            _sections[i].train->target_speed = min(val, MOTOR_DRV_OUT_MIN);
          }
          break;
      }
    }
    //出力更新
    /*
    区間状態が
      [空き/前の区間に列車なし(SECTION_EMPTY)]->出力なし
      [空き/前の区間に列車あり(SECTION_PREPARE)]->出力あり
      [在線/入口踏んでいる(SECTION_ENTERING)]->出力あり
      [在線/完全に入った(SECTION_ENTERED)]->出力あり
      [在線/出口踏んでいる(SECTION_EXITING)]出力あり
    */
    if(_sections[i].is_occupied != SECTION_EMPTY && _sections[i].train != NULL){
      *_sections[i].fwd_pin = _sections[i].train->current_speed;
      *_sections[i].rev_pin = 0;
    }else{
      //ショートブレーキにしてあるものの、ハイインピーダンスのほうが冒進時に安全
      *_sections[i].fwd_pin = 0;
      *_sections[i].rev_pin = 0;
    }
  }
}
