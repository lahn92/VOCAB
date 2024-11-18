#include <Arduino.h>
#include "WindVaneSensor.h"

void readWindVaneSensor(unsigned long currentMillis, bool debugMode) {
    static unsigned long lastUpdateTime = 0;
    if (currentMillis - lastUpdateTime >= 500) {
        lastUpdateTime = currentMillis;
        int sensorValue = analogRead(A2); // Change from A1 to A2
        float voltage = sensorValue * (3.3 / 1023.0);
        String direction = getDirection(voltage);
        if (debugMode) {
            Serial.print("Sensor Value: ");
            Serial.print(sensorValue);
            Serial.print(", Voltage: ");
            Serial.print(voltage);
            Serial.print(", Direction: ");
            Serial.println(direction);
        }
    }
}

String getDirection(float voltage) {
    float tolerance = 0.1;
    if (abs(voltage - 0.75) < tolerance) {
        return "N";
    } else if (abs(voltage - 1.83) < tolerance) {
        return "NE";
    } else if (abs(voltage - 2.96) < tolerance) {
        return "E";
    } else if (abs(voltage - 2.66) < tolerance) {
        return "SE";
    } else if (abs(voltage - 2.4) < tolerance) {
        return "S";
    } else if (abs(voltage - 1.24) < tolerance) {
        return "SW";
    } else if (abs(voltage - 0.25) < tolerance) {
        return "W";
    } else if (abs(voltage - 0.41) < tolerance) {
        return "NW";
    } else {
        return "Unknown(Error)";
    }
}
