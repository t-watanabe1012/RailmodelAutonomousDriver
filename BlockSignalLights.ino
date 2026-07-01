/*
atmega 2560(Arduino MEGA2560)を使用

概要
入口にその区間に列車が存在しているかに合わせて点灯するLEDの色が変わるシステム

詳細
・扱う区間は4個で、それらをS1,S2,S3,S4とする.
区間はS1->S2->S3->S4->S1とループ状に接続されている.
・各区間の入口には赤・緑のLEDと列車検出用のセンサがおかれている.
・各区間の中間にもセンサがおかれている.
・各区間の入口にあるLEDは、その区間の入口センサが踏まれてから、次の区間の中間センサが踏まれるまでの間、赤色が点灯し、緑色は消灯する。
それ以外のタイミングでは、緑色が点灯し、赤色は消灯する. 

列車検出機構
・センサは赤外線によるフォトリフレクタを使って列車の通過を検出する。
・フォトトランジスタに赤外線を検出した場合、センサの出力はLOWとなる。
・センサはON/OFFを繰り返す。1サイクルのON/OFFの間でONの時にLOW、OFFの間にHIGHとなっている場合を列車通過と判定する。
・ON/OFFサイクルはON:1ms,OFF:9msとし、100Hzとする. 
鉄道模型車両が10msで進める距離は8mm程度であり、1両当たり約14cmなので速度による検出漏れの可能性は低い.
・一度、センサーが踏まれていると判断した後、PASSING_INTERVAL_MSミリ秒間は通過中として、センサーが踏まれているものとする
実験的に500で行っている。車両の特徴によって変更してもよいかも。

接続ピン
・センサ電源 : 1(Pch mosfetを動かす)->38(PORTD7)を使用
・センサ信号線 : 8(PORTA0~7, 22~29を使用)
・LED : 8(PORTC0~7, 37~30を使用)
17線
*/
//センサーの状態を表す:非検知, 検知
typedef enum {
  NOT_DETECTED, DETECTED
}SensorState;

//センサーが一度検出した後、下記の時間は列車が直上を通過中とみなす
#define PASSING_INTERVAL_MS 500
//構造体:センサーを表す
typedef struct {
  byte pin;
  SensorState state;
  //直近で踏まれた時間を記録する
  unsigned long latest_passed_ms;
}Sensor;

//センサ変数の命名規則:[区間][入口は0/中間は1]->A0, A1, ..., D0, D1
Sensor sensorA0 = {0, NOT_DETECTED, 0};
Sensor sensorA1 = {1, NOT_DETECTED, 0};
Sensor sensorB0 = {2, NOT_DETECTED, 0};
Sensor sensorB1 = {3, NOT_DETECTED, 0};
Sensor sensorC0 = {4, NOT_DETECTED, 0};
Sensor sensorC1 = {5, NOT_DETECTED, 0};
Sensor sensorD0 = {6, NOT_DETECTED, 0};
Sensor sensorD1 = {7, NOT_DETECTED, 0};
//現状、中間センサは使っていない
//入口センサで次の信号を確認して目標速度を設定する
//区間がもっと長くなった場合は中間センサも使用する

//センサーの個数を表す
#define SENSOR_NO 8

typedef enum{
  IDLE = 0,
    ON = 1,
   OFF = 2
}SensorManagerState;

//センサーの管理をするための構造体
typedef struct {
  Sensor* sensors[SENSOR_NO];
  int ON_pin;
  SensorManagerState state;
}SensorManager;

SensorManager sensorMaster;
//センサーのピンの入出力を設定する
void init_for_sensorManager(){
  sensorMaster.sensors[0] = &sensorA0;
  sensorMaster.sensors[1] = &sensorA1;
  sensorMaster.sensors[2] = &sensorB0;
  sensorMaster.sensors[3] = &sensorB1;
  sensorMaster.sensors[4] = &sensorC0;
  sensorMaster.sensors[5] = &sensorC1;
  sensorMaster.sensors[6] = &sensorD0;
  sensorMaster.sensors[7] = &sensorD1;

  sensorMaster.ON_pin = PD7;
  sensorMaster.state  = IDLE;
  for(int i = 0; i < SENSOR_NO; i ++){
    //センサーの信号線はPORTAに接続する予定なので、入力にし、内部プルアップを有効とする
    DDRA  &=  ~(1 << sensorMaster.sensors[i]->pin);
    PORTA |=   (1 << sensorMaster.sensors[i]->pin);
  }
  //センサーの電源を一括OFFにして待機する
   DDRD |=  (1 << sensorMaster.ON_pin);
  PORTD &= ~(1 << sensorMaster.ON_pin);
}

//区間の状態を表す
typedef enum {
  SECTION_EMPTY   = 0,  //在線なし
  SECTION_OCCPIED = 1,  //在線あり
  SECTION_EXITING = 2,  //在線あり(出かかっている)
  SECTION_PREPARE = 3,  //在線なし(受け入れ準備)
}SectionState;

//チェック間隔(ms)秒単位
const unsigned long CHECK_INTERVAL_ms = 8UL;
//LEDの点灯・消灯から計測まで待つ時間(ms単位)
const unsigned long STABLE_INTERVAL_ms = 1UL;

unsigned long previous_ms = 0;
unsigned long current_ms  = 0;
uint8_t LED_ON_sensor_val;
uint8_t LED_OFF_sensor_val;
uint8_t sensor_sesult;

void sensorCheck(){
  switch(sensorMaster.state){
    case IDLE :
      if(current_ms - previous_ms >= CHECK_INTERVAL_ms){
        previous_ms = current_ms;
        //センサーのLEDを点灯させる//LOWで点灯になる点に注意
        PORTD &= ~(1 << sensorMaster.ON_pin);
        sensorMaster.state  = ON;
      }
      break; 
    case   ON :
      if(current_ms - previous_ms >= STABLE_INTERVAL_ms){
        previous_ms = current_ms;
        //各ピンの状態を記録する->フォトトランジスタ導通時(外光の影響を受けているまたは踏まれたとき)はLOWなので、反転して記録する
        LED_ON_sensor_val = ~PINA;
        //センサーのLEDを消灯させる
        PORTD |= (1 << sensorMaster.ON_pin);
        sensorMaster.state  = OFF;
      }
      break; 
    case  OFF :
      if(current_ms - previous_ms >= STABLE_INTERVAL_ms){
        previous_ms = current_ms;
        //各ピンの状態を記録する->外光の影響を受けていないときはHIGH
        LED_OFF_sensor_val = PINA;
        //点灯時の状態と合わせて最終判定とする
        sensor_sesult = LED_ON_sensor_val & LED_OFF_sensor_val;
        //センサーのLEDを消灯させる
        PORTD |= (1 << sensorMaster.ON_pin);
        sensorMaster.state  = IDLE;
        //センサーが踏まれていると判定した場合、そのセンサのlatest_passed_timeを更新する
        byte mask = 0B00000001;
        for(int i = 0; i < SENSOR_NO; i ++){
          if(sensor_sesult & mask){
            sensorMaster.sensors[i]->state = DETECTED;
            sensorMaster.sensors[i]->latest_passed_ms = current_ms;
          }
          mask = mask << 1;
        }
      }
      break;
    default : 
      break;
  }
  //最後に踏まれた時刻から一定時間経過後にセンサーの通過判定解除
  for(int i = 0; i < SENSOR_NO; i ++){
    if(current_ms - sensorMaster.sensors[i]->latest_passed_ms >= PASSING_INTERVAL_MS){
      sensorMaster.sensors[i]->state = NOT_DETECTED;
    }
  }
}

//信号機の状態(点灯している色)を表す
/*RE:停止(赤1個)・YY:警戒(黄色2個)・YE:注意(黄色1個)・YG:減速(黄色1個・緑1個)・GR:進行(緑1個)*/
typedef enum{
  RE, YY, YE, YG, GR
}SignalLightsStatus;

//信号機を表す構造体(今回は2灯のみに対応)
typedef struct {
  SignalLightsStatus status;
  int re_pin;
  int gr_pin;
}SignalLight;

//pin番号にはPORTCでの番号を入れておく
SignalLight signalLightA = {GR, 7, 6};
SignalLight signalLightB = {GR, 5, 4};
SignalLight signalLightC = {GR, 3, 2};
SignalLight signalLightD = {GR, 1, 0};

#define SIGNALLight_NUM 4
SignalLight* signalLights[SIGNALLight_NUM]={&signalLightA, &signalLightB, &signalLightC, &signalLightD};

void init_for_signalLights(){
  int redPin;
  int grnPin;
  for(int i = 0; i < SIGNALLight_NUM; i ++){
    redPin = signalLights[i]->re_pin;
    grnPin = signalLights[i]->gr_pin;
    //信号機のピンはPORTCにつなぐ予定
    DDRC  |=   (1 << redPin) | (1 << grnPin); 
    PORTC &= ~((1 << redPin) | (1 << grnPin)); 
  }
}

//分周比1として出力を400とすると20kHzのPWMとできる
#define MOTOR_DRV_MAX 400;
//
typedef struct{
  //目標スピード
  uint16_t target_speed;
  //現在スピード
  uint16_t current_speed;
}Train;

//1本目の列車
Train first_train = {0, 0};
//2本目の列車
Train second_train = {0, 0};
typedef struct Section Section;
//各区間を表す構造体
/*区間と信号機・センサーの対応を紐付けておく*/
struct Section {
  //この区間に列車が在線(出口未到着)・在線(出口到着中)・非在線
  SectionState is_occpied;
  //この区間入口の信号機
  SignalLight* entrance_signalLights;
  //前の区間
  Section* prev_section;
  //次の区間
  Section* next_section;
  //入口センサー:この区間の入口センサ
  Sensor* entrance_sensor;
  //中間センサー:この区間の中間センサ
  Sensor* middle_sensor;
  //出口センサー:次の区間の入口センサ
  Sensor* exit_sensor;
  //モータードライバの前進ピンの設定
  volatile uint16_t* fwd_pin;
  //モータードライバの後退ピンの設定
  volatile uint16_t* rev_pin;
  //列車がいる場合、そのアドレスを持っておく(列車の現在出力しか使わないがおいておく)
  Train* train;
};

#define SECTION_NUM 4
extern Section sectionA;
extern Section sectionB;
extern Section sectionC;
extern Section sectionD;
Section sectionA = {SECTION_OCCPIED, &signalLightA, &sectionD, &sectionB, &sensorA0, &sensorA1, &sensorB0, &OCR3A, &OCR3B, &first_train};
Section sectionB = {  SECTION_EMPTY, &signalLightB, &sectionA, &sectionC, &sensorB0, &sensorB1, &sensorC0, &OCR4A, &OCR3C, NULL};
Section sectionC = {  SECTION_EMPTY, &signalLightC, &sectionB, &sectionD, &sensorC0, &sensorC1, &sensorD0, &OCR4B, &OCR5A, NULL};
Section sectionD = {SECTION_OCCPIED, &signalLightD, &sectionC, &sectionA, &sensorD0, &sensorD1, &sensorA0, &OCR4C, &OCR5B, &second_train};

Section* sections[SECTION_NUM] = {&sectionA, &sectionB, &sectionC, &sectionD};

void init_for_sections(){
  /*セクションごとに異なるタイマーを使うので、同期操作が必須。
  同期の手順
  タイマー停止->レジスタ書き込み->カウンタを0にリセット->タイマー再開
  */
  //タイマーの設定を変える前にタイマーを止める
  GTCCR = (1 << TSM);
  /*タイマー3～5の各ピンをphase collect PWMで動作させる
    分周比は8, TOPはICRnとする。 
    phese collect PWMは同期の取りやすさが特徴である。phase collect PWMではOCRnXの値更新を必ず0かTOPまで待ってくれる。どちらで待つかは設定による。
    これによりモータードライバの短絡防止につながる。
  */
  TCCR3A = 0B11111100;
  TCCR3B = 0B00010001;
  //タイマー3のピンはE3～E5(PE3=5, PE4=2, PE5=3)なので、これらを出力に設定する
  DDRE |= (1 << PE3) | (1 << PE4) | (1 << PE5); 
  ICR3 = MOTOR_DRV_MAX;
  TCCR4A = 0B11111100;
  TCCR4B = 0B00010001;
  //タイマー4のピンはH3～H5(PH3=6, PH4=7, PH5=8)なので、これらを出力に設定する
  DDRH |= (1 << PH3) | (1 << PH4) | (1 << PH5); 
  ICR4 = MOTOR_DRV_MAX;
  TCCR5A = 0B11111100;
  TCCR5B = 0B00010001;
  //タイマー5のピンはL3～L5(PL3=46, PL4=45, PL5=44)なので、これらを出力に設定する
  DDRL |= (1 << PL3) | (1 << PL4) | (1 << PL5); 
  ICR5 = MOTOR_DRV_MAX;
  //カウンタリセット
  TCNT3 = 0;
  TCNT4 = 0;
  TCNT5 = 0;
  //プリスケーラーもリセット(TSM:タイマー同期)
  GTCCR = (1 << TSM) | (1 << PSRSYNC);
  //タイマーを再開
  GTCCR = 0B00000000;
}

void updateSectionState(){
  for(int i = 0; i < SECTION_NUM; i ++){
    //入口センサが踏まれたらこの区間に列車がいることになる
    if(sections[i]->entrance_sensor->state == DETECTED){
      sections[i]->is_occpied = SECTION_OCCPIED;
    }
    //出口センサが踏まれたらこの区間から列車が出て行っていることになる
    if(sections[i]->exit_sensor->state == DETECTED){
      sections[i]->is_occpied = SECTION_EXITING;
    }
    //出て行っている状態で出口センサが踏まれていなければこの区間から完全に出て行ったことになる
    if(sections[i]->is_occpied == SECTION_EXITING && sections[i]->exit_sensor->state == NOT_DETECTED){
      sections[i]->is_occpied = SECTION_EMPTY;
      //列車がいなくなったのでtrainをNULLにする
      sections[i]->train = NULL;
    }
    //この区間に列車がなく、前の区間に列車がいる場合
    if(sections[i]->is_occpied == SECTION_EMPTY && sections[i]->prev_section->is_occpied == SECTION_OCCPIED){
      sections[i]->is_occpied = SECTION_PREPARE;
      //いなくなったのでtrainをNULLにする
      sections[i]->train = sections[i]->prev_section->train;
    }
    //信号の現示内容更新
    switch(sections[i]->is_occpied){
      case SECTION_EMPTY : 
      case SECTION_PREPARE : 
        sections[i]->entrance_signalLights->status = GR;
        break;
      case SECTION_EXITING : 
      case SECTION_OCCPIED : 
      default : 
        sections[i]->entrance_signalLights->status = RE;
        break;
    }
    //信号の現示更新
    switch(sections[i]->entrance_signalLights->status){
      case GR: 
        PORTC |= (1 << sections[i]->entrance_signalLights->gr_pin);
        PORTC &=~(1 << sections[i]->entrance_signalLights->re_pin);
        break;
      case RE: 
      default :
        PORTC |= (1 << sections[i]->entrance_signalLights->re_pin);
        PORTC &=~(1 << sections[i]->entrance_signalLights->gr_pin);
        break;
    }
    //列車の最高速度更新
    int val = analogRead(A0)/4;
    /*在線/出口踏んでいないときのみ処理*/
    if(sections[i]->is_occpied == SECTION_OCCPIED){
      //前に列車がいるかどうかで最高速度を変える
      switch(sections[i]->next_section->is_occpied){
        //前に列車がない
        case SECTION_EMPTY : 
        case SECTION_PREPARE : 
          if(sections[i]->train != NULL){
            //OCRxnの範囲は0~400だが、280で最大出力を70%程度にしておく。
            sections[i]->train->target_speed = min(val, 280);
          }
          break;
        //前に列車がある
        case SECTION_OCCPIED :
        case SECTION_EXITING :
        default : 
          if(sections[i]->train != NULL){
            sections[i]->train->target_speed = min(val, 30);
          }
          break;
      }
    }
    //出力更新
    /*
    区間状態が
      [空き/前の区間に列車なし(SECTION_EMPTY)]->出力なし
      [空き/前の区間に列車あり(SECTION_PREPARE)]->出力あり
      [在線/出口踏んでいない(SECTION_OCCUPIED)]->出力あり
      [在線/出口踏んでいる(SECTION_EXITING)]出力あり
    */
    if(sections[i]->is_occpied != SECTION_EMPTY && sections[i]->train != NULL){
      *sections[i]->fwd_pin = sections[i]->train->current_speed;
      *sections[i]->rev_pin = 0;
    }else{
      //ショートブレーキにしてあるものの、ハイインピーダンスのほうが冒進時に安全
      *sections[i]->fwd_pin = 0;
      *sections[i]->rev_pin = 0;
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  init_for_sensorManager();
  init_for_signalLights();
  init_for_sections();
  
}

unsigned long previous_acc_ms = 0;
unsigned long previous_blk_ms = 0;
void loop() {
  // put your main code here, to run repeatedly:
  current_ms = millis();
  sensorCheck();
  updateSectionState();
  //10msごとに加速
  if(current_ms - previous_acc_ms >= 10){
    previous_acc_ms = current_ms;
    if(first_train.target_speed > first_train.current_speed){first_train.current_speed ++;}
    if(second_train.target_speed > second_train.current_speed){second_train.current_speed ++;}  
  }
  //20msごとに減速
  if(current_ms - previous_blk_ms >= 20){
    previous_blk_ms = current_ms;
    if(first_train.target_speed < first_train.current_speed){first_train.current_speed --;}
    if(second_train.target_speed < second_train.current_speed){second_train.current_speed --;}
  }
}
