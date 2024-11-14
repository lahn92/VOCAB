  #include <Arduino.h>
  #include "WindVaneSensor.h"

  void readWindVaneSensor(unsigned long currentMillis) {
    static unsigned long lastUpdateTime = 0;
    if (currentMillis - lastUpdateTime >= 500) {
      lastUpdateTime = currentMillis;
      int sensorValue = analogRead(A1);
      float voltage = sensorValue * (3.3 / 1023.0);
      String direction = getDirection(voltage);
      Serial.print("Sensor Value: ");
      Serial.print(sensorValue);
      Serial.print(", Voltage: ");
      Serial.print(voltage);
      Serial.print(", Direction: ");
      Serial.println(direction);
    }
  }

  String getDirection(float voltage) {
    float tolerance = 0.1;
    if (abs(voltage - 0.49) < tolerance) {
      return "N";
    } else if (abs(voltage - 1.16) < tolerance) {
      return "NE";
    } else if (abs(voltage - 1.94) < tolerance) {
      return "E";
    } else if (abs(voltage - 1.75) < tolerance) {
      return "SE";
    } else if (abs(voltage - 1.53) < tolerance) {
      return "S";
    } else if (abs(voltage - 0.81) < tolerance) {
      return "SW";
    } else if (abs(voltage - 0.15) < tolerance) {
      return "W";
    } else if (abs(voltage - 0.27) < tolerance) {
      return "NW";
    } else {
      return "Unknown(Error)";
    }
  }
