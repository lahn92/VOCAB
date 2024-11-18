#ifndef WINDVANESENSOR_H
#define WINDVANESENSOR_H

#include <Arduino.h>

void readWindVaneSensor(unsigned long currentMillis, bool debugMode);
String getDirection(float voltage);

#endif
