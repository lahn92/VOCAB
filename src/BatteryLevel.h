#ifndef BATTERY_LEVEL_H
#define BATTERY_LEVEL_H

#include <Arduino.h>

// Define pin constants
const int analogPin = A4;
const int ledPins[] = {39, 41, 43, 45, 47};

// Conversion factor
const float conversionFactor = 0.0248;

// Variables for moving average
const int numReadings = 1000;
extern float readings[numReadings];
extern int readIndex;
extern float total;
extern float average;

// Standard deviation calculation
extern float stdDev;

// Timing for blinking
extern unsigned long previousMillis;
const long blinkInterval = 500;
extern bool ledState;

// Timing for button debounce
extern unsigned long buttonPressMillis;
const long debounceTime = 50;

// Timing for LED display
extern unsigned long displayMillis;
const long displayDuration = 5000;
const long startupDisplayDuration = 10000000; // 10 seconds

// Button state variables
extern bool lastButtonState;
extern bool buttonState;
extern bool displayBatteryLevel;
extern bool startupDisplay;

// Debugging flag
extern bool debugEnabled;

void setupBatteryLevel();
void loopBatteryLevel();
void setLEDs(float numLit, unsigned long currentMillis, long interval, bool allBlink);
void enableDebug();
void disableDebug();
void handleSerialInput();

#endif // BATTERY_LEVEL_H
