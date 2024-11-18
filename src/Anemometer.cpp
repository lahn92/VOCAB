#include "Anemometer.h"

Anemometer::Anemometer(int pin) 
    : anemometerPin(pin), pulseCount(0), analogValue(0), lastAnalogValue(0), previousMillis(0), pulseDetected(false), debugMode(false), totalWindSpeed(0), measurementCount(0), startMillis(0) {}

void Anemometer::setup() {
    pinMode(anemometerPin, INPUT);
    //Serial.begin(115200);
    resetAverageCalculation();
}

void Anemometer::update() {
    unsigned long currentMillis = millis();

    // Sample the analog pin
    samplePin();

    // Check if 1 second has passed
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        
        // Calculate wind speed
        float windSpeed = pulseCount * speedFactor;
        totalWindSpeed += windSpeed;
        measurementCount++;

        if (debugMode) {
            Serial.print("Wind Speed: ");
            Serial.print(windSpeed);
            Serial.println(" m/s");

            // Print current average wind speed
            Serial.print("Current Average Wind Speed (last 10 minutes): ");
            Serial.println(getAverageWindSpeed());

            // Print pulses per second
            Serial.print("Pulses per second: ");
            Serial.println(pulseCount);

            // Reset average calculation every 10 minutes
            if (currentMillis - startMillis >= 600000) { // 600000 milliseconds = 10 minutes
                resetAverageCalculation();
            }
        }

        // Reset the pulse count for the next second
        pulseCount = 0;
    }
}

void Anemometer::samplePin() {
    analogValue = analogRead(anemometerPin); // Read the analog value from pin

    // Check for a rising edge (low to high transition)
    if (analogValue > threshold && lastAnalogValue <= threshold) {
        pulseCount++;
    }

    // Update the last analog value
    lastAnalogValue = analogValue;
}

float Anemometer::getWindSpeed() {
    return pulseCount * speedFactor;
}

float Anemometer::getAverageWindSpeed() {
    if (measurementCount == 0) return 0;
    return totalWindSpeed / measurementCount;
}

void Anemometer::setDebug(bool debug) {
    debugMode = debug;
}

void Anemometer::resetAverageCalculation() {
    totalWindSpeed = 0;
    measurementCount = 0;
    startMillis = millis();
}
