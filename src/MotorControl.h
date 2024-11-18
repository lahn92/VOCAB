#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H

#include <Servo.h>

extern bool debugMode;

void setupMotorControl();
void driveRobot();
void setThrust(unsigned int thrustA, unsigned int thrustB);
bool isControllerConnected();

#endif
