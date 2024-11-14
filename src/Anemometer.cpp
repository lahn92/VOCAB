#include "Anemometer.h"

Anemometer::Anemometer(int pin, int numReadings) 
    : _pin(pin), _numReadings(numReadings), _currentReadingIndex(0), _sum(0), _directReading(0), _movingAverage(0) {
    _readings = new float[numReadings];
    for (int i = 0; i < numReadings; ++i) {
        _readings[i] = 0;
    }
}

Anemometer::~Anemometer() {
    delete[] _readings;
}

void Anemometer::update() {
    // Read the voltage from the anemometer
    float voltage = analogRead(_pin) * (3.3 / 1023.0);
    _directReading = voltageToSpeed(voltage);

    // Update the moving average
    _sum -= _readings[_currentReadingIndex];
    _readings[_currentReadingIndex] = _directReading;
    _sum += _directReading;

    _currentReadingIndex = (_currentReadingIndex + 1) % _numReadings;
    _movingAverage = _sum / _numReadings;
}

float Anemometer::getDirectReading() {
    return _directReading;
}

float Anemometer::getMovingAverage() {
    return _movingAverage;
}


void Anemometer::logReadingsToFile(const char* filename, bool serialEnabled) {
    unsigned long time = millis();  // Use millis() as a placeholder for GNSS time
    char buffer[100];
    sprintf(buffer, "Time: %lu, Direct: %.2f m/s, Moving Avg: %.2f m/s", time, _directReading, _movingAverage);
    if (serialEnabled) {
        Serial.println(buffer);
    }

    FILE* file = fopen(filename, "a");
    if (file != NULL) {
        fprintf(file, "%s\n", buffer);
        fclose(file);
    } else {
        if (serialEnabled) {
            Serial.println("Failed to open file!");
        }
    }
}


float Anemometer::voltageToSpeed(float voltage) {
    // Convert voltage to wind speed
    return (voltage - _voltageMin) / (_voltageMax - _voltageMin) * (_speedMax - _speedMin) + _speedMin;
}
