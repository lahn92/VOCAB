#include "WaterSensor.h"

WaterSensor::WaterSensor(int pin1, int pin2) : _pin1(pin1), _pin2(pin2) {}

bool WaterSensor::check(bool serialEnabled) {
    int sensorValue1 = analogRead(_pin1);
    int sensorValue2 = analogRead(_pin2);
    String status = getStatus(sensorValue1, 1) + " - " + getStatus(sensorValue2, 2);

    if (serialEnabled) {
        Serial.println(status + " - Sensor 1 Value: " + String(sensorValue1) + " - Sensor 2 Value: " + String(sensorValue2));
    }

    return (sensorValue1 < _thresholdLow || sensorValue2 < _thresholdLow);
}

String WaterSensor::getStatus(int sensorValue, int sensorNumber) {
    if (sensorValue < _thresholdLow) return "Water detected by Sensor " + String(sensorNumber) + "!";
    else if (sensorValue < _thresholdHigh) return "High humidity detected by Sensor " + String(sensorNumber) + "!";
    else return "No water detected by Sensor " + String(sensorNumber);
}
