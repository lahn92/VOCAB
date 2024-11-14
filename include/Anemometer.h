#ifndef ANEMOMETER_H
#define ANEMOMETER_H

#include <Arduino.h>
#include "GNSSHandler.h"  // Include GNSSHandler instead of GPSSystem

class Anemometer {
  public:
    Anemometer(int pin, int numReadings = 10);
    ~Anemometer();
    void update();
    float getDirectReading();
    float getMovingAverage();
    void logReadingsToFile(const char* filename, bool serialEnabled);

  private:
    int _pin;
    int _numReadings;
    int _currentReadingIndex;
    float *_readings;
    float _sum;
    float _directReading;
    float _movingAverage;
    const float _voltageMin = 0.4;
    const float _voltageMax = 2.0;
    const float _speedMin = 0.0;
    const float _speedMax = 32.0;
    float voltageToSpeed(float voltage);
};

#endif // ANEMOMETER_H
