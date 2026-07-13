#include <RailwayModels.h>

Sensor sensors[6] = {Sensor(0), Sensor(1), Sensor(2), Sensor(3), Sensor(4), Sensor(5),};
SensorManager sensorManager = SensorManager(sensors);

unsigned long previous_ms = 0;
unsigned long current_ms  = 0;

Section sections[6] = {
  Section(sections[5], sections[1], sensors[5], sensors[0], OCR1A, OCR1B),//区間0
  Section(sections[0], sections[2], sensors[0], sensors[1], OCR1C, OCR3A),//区間1
  Section(sections[1], sections[3], sensors[1], sensors[2], OCR3B, OCR3C),//区間2
  Section(sections[2], sections[4], sensors[2], sensors[3], OCR4A, OCR4B),//区間3
  Section(sections[3], sections[5], sensors[3], sensors[4], OCR4C, OCR5A),//区間4
  Section(sections[4], sections[0], sensors[4], sensors[5], OCR5B, OCR5C),//区間5
};
SectionManager section_manager = SectionManager(sections);

SignalLight signalLights[6] = {
  SignalLight(sections[0]),
  SignalLight(sections[1]),
  SignalLight(sections[2]),
  SignalLight(sections[3]),
  SignalLight(sections[4]),
  SignalLight(sections[5]),
};
SignalLightManager signalLightManager = SignalLightManager(signalLights);

Train trains[2] = {Train(),Train()};
TrainManager trainManager = TrainManager(trains);

void setup(){
  section_manager.init_for_sections();
  signalLightManager.init_for_signalLightManager();
  trains[0].putTrain(sections[0]);
  trains[1].putTrain(sections[4]);
}

void loop(){
  current_ms = millis();
  sensorManager.sensorCheck();
  section_manager.updateSectionState();
  trainManager.trainSpdControl();
  signalLightManager.signalLights_update();
}
