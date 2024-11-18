#include <Arduino.h>
#include "MotorControl.h"
#include "StateMachine.h"  // Include for state management

Servo escA;
Servo escB;
const int throttlePinA = 5;
const int throttlePinB = 6;

int currentThrottleA = 1500;
int currentThrottleB = 1500;
unsigned long lastMillis = 0;
const unsigned long rampInterval = 100;
const int throttleIncrement = 5;

unsigned long lastSignalTimeA = 0;
unsigned long lastSignalTimeB = 0;
const unsigned long signalTimeout = 10000;  // Timeout period in milliseconds

void setupMotorControl() {
  escA.attach(throttlePinA);
  escB.attach(throttlePinB);
}

void driveRobot() {
  unsigned long leftright = pulseIn(2, HIGH);
  unsigned long fremtilbage = pulseIn(3, HIGH);

  //if (leftright > 1470 && leftright < 1530) {
  //  leftright = 1495;
  //}
  //if (fremtilbage > 1470 && fremtilbage < 1530) {
  //  fremtilbage = 1495;
  //}

  const float alpha = 0.05;
  static float filteredleftright = 1500;
  static float filteredfremtilbage = 1500;
  filteredleftright = alpha * leftright + (1 - alpha) * filteredleftright;
  filteredfremtilbage = alpha * fremtilbage + (1 - alpha) * filteredfremtilbage;

  const int MIN_PWM = 995;
  const int MAX_PWM = 1995;

  int thrust = map(filteredfremtilbage, MIN_PWM, MAX_PWM, -100, 100);
  int difference = map(filteredleftright, MIN_PWM, MAX_PWM, -100, 100);
  int thrustA = thrust + difference;
  int thrustB = thrust - difference;

  thrustA = constrain(thrustA, -100, 100);
  thrustB = constrain(thrustB, -100, 100);

  setThrust(thrustA, thrustB);

  if (thrustA != 0) {
    lastSignalTimeA = millis();
  }
  if (thrustB != 0) {
    lastSignalTimeB = millis();
  }

  if (debugThrust) {  // Ensure debugThrust is properly referenced
    Serial.print("Thrust A: ");
    Serial.print(thrustA);
    Serial.print(", Thrust B: ");
    Serial.println(-thrustB);
  }
}

void setThrust(unsigned int thrustA, unsigned int thrustB) {
  if (thrustA >= -2 && thrustA <= 2) {
    thrustA = 0;
  }
  if (thrustB >= -2 && thrustB <= 2) {
    thrustB = 0;
  }

  int targetThrottleA = map(thrustA, 100, -100, 1410, 1610);
  int targetThrottleB = map(thrustB, 100, -100, 1410, 1610);

  unsigned long currentMillis = millis();

  if (currentMillis - lastMillis >= rampInterval) {
    if (abs(currentThrottleA - targetThrottleA) <= throttleIncrement) {
      currentThrottleA = targetThrottleA;
    } else if (currentThrottleA < targetThrottleA) {
      currentThrottleA += throttleIncrement;
    } else if (currentThrottleA > targetThrottleA) {
      currentThrottleA -= throttleIncrement;
    }

    if (abs(currentThrottleB - targetThrottleB) <= throttleIncrement) {
      currentThrottleB = targetThrottleB;
    } else if (currentThrottleB < targetThrottleB) {
      currentThrottleB += throttleIncrement;
    } else if (currentThrottleB > targetThrottleB) {
      currentThrottleB -= throttleIncrement;
    }

    escA.writeMicroseconds(currentThrottleA);
    escB.writeMicroseconds(currentThrottleB);
    lastMillis = currentMillis;
  }
  if (debugThrust) {  // Ensure debugThrust is properly referenced
   Serial.println(currentThrottleA);
   Serial.println(currentThrottleB);
  }
}

bool isControllerConnected() {
  unsigned long currentMillis = millis();
  bool connectedA = (currentMillis - lastSignalTimeA) < signalTimeout;
  bool connectedB = (currentMillis - lastSignalTimeB) < signalTimeout;

  return connectedA && connectedB;
}
