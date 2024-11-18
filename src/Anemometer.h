#ifndef ANEMOMETER_H
#define ANEMOMETER_H

#include <Arduino.h>

class Anemometer {
public:
    Anemometer(int pin);
    void setup();
    void update();
    float getWindSpeed();
    float getAverageWindSpeed();
    void setDebug(bool debug);

private:
    const int anemometerPin;
    const int threshold = 512; // Adjust this value according to your needs
    const unsigned long interval = 1000; // 1-second interval
    const float speedFactor = 0.66; // Factor to convert pulses to wind speed

    volatile int pulseCount; // Declare pulseCount as volatile
    int analogValue;
    int lastAnalogValue;
    unsigned long previousMillis;
    bool pulseDetected;
    bool debugMode;

    void samplePin(); // Function to sample the analog pin

    // Variables for calculating the average wind speed over 10 minutes
    float totalWindSpeed;
    int measurementCount;
    unsigned long startMillis;

    void resetAverageCalculation(); // Function to reset the average calculation
};

#endif
