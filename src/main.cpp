#include <Arduino.h>
#include "StateMachine.h"
#include "MotorControl.h"
#include "GNSSHandler.h"
#include "WindVaneSensor.h"
#include "Haversine.h"
#include "WaterSensor.h"
#include "Anemometer.h"  // Include Anemometer

WaterSensor waterSensor(A0, A1);  // Instantiate the WaterSensor with appropriate pins
Anemometer anemometer(A2);  // Instantiate the Anemometer with appropriate pin

void setup() {
  setupMotorControl();
  setupGNSS();
  Serial.begin(115200);
  printCurrentState();
}

void loop() {
  unsigned long currentMillis = millis();
  checkSerialCommands();
  checkStateTransition();
  updateStateMachine(currentMillis);
  printCurrentStatePeriodically(currentMillis);
  
  if (debugWater) {
    waterSensor.check(true);  // Check the water sensor with serial output enabled in debug mode
  } else {
    waterSensor.check(false);  // Check the water sensor with serial output disabled
  }

  if (debugAnemometer) {
    anemometer.update();  // Update the anemometer readings
    Serial.print("Anemometer Direct Reading: ");
    Serial.print(anemometer.getDirectReading());
    Serial.print(" m/s, Moving Average: ");
    Serial.println(anemometer.getMovingAverage());
  } else {
    anemometer.update();  // Update the anemometer readings without printing
  }
}
