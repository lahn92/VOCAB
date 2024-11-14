#ifndef WATER_SENSOR_H
#define WATER_SENSOR_H

#include <Arduino.h>

class WaterSensor {
  public:
    WaterSensor(int pin1, int pin2);
    bool check(bool serialEnabled);

  private:
    int _pin1, _pin2;
    const int _thresholdLow = 300;
    const int _thresholdHigh = 500;
    String getStatus(int sensorValue, int sensorNumber);
};

#endif // WATER_SENSOR_H
