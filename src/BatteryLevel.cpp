#include "BatteryLevel.h"

// Variables for moving average
float readings[numReadings];
int readIndex = 0;
float total = 0;
float average = 0;

// Standard deviation calculation
float stdDev = 0;

// Timing for blinking
unsigned long previousMillis = 0;
bool ledState = LOW;

// Timing for button debounce
unsigned long buttonPressMillis = 0;

// Timing for LED display
unsigned long displayMillis = 0;

// Button state variables
bool lastButtonState = LOW;
bool buttonState = LOW;
bool displayBatteryLevel = false;
bool startupDisplay = true;

// Debugging flag
bool debugEnabled = false;

void setupBatteryLevel() {
    //Serial.begin(115200);
    pinMode(PC_13, INPUT);
    for (int i = 0; i < 5; i++) {
        pinMode(ledPins[i], OUTPUT);
    }
    for (int i = 0; i < numReadings; i++) {
        readings[i] = 0;
    }

    // Start displaying battery level for 15 seconds on startup
    displayBatteryLevel = true;
    displayMillis = millis();
}

void loopBatteryLevel() {
    handleSerialInput();  // Handle serial input for debugging commands
    unsigned long currentMillis = millis();

    // Calculate moving average and standard deviation
    total = total - readings[readIndex];
    readings[readIndex] = analogRead(analogPin) * conversionFactor - 0.06;
    total = total + readings[readIndex];
    readIndex = (readIndex + 1) % numReadings;
    average = total / numReadings;

    float sum = 0;
    for (int i = 0; i < numReadings; i++) {
        sum += sq(readings[i] - average);
    }
    stdDev = sqrt(sum / numReadings);

    // Check button state
    bool reading = digitalRead(PC_13);

    if (reading != lastButtonState && (currentMillis - buttonPressMillis) > debounceTime) {
        buttonPressMillis = currentMillis;
        if (reading == HIGH) {
            displayBatteryLevel = true;
            displayMillis = currentMillis;
        }
    }

    lastButtonState = reading;

    // LED display logic
    float numLit = 0;
    bool allBlink = false;

    if (average > 25.25) {
        numLit = 5;
    } else if (average > 24.75) {
        numLit = 4.5;
    } else if (average > 24.25) {
        numLit = 4;
    } else if (average > 23.75) {
        numLit = 3.5;
    } else if (average > 23.25) {
        numLit = 3;
    } else if (average > 22.75) {
        numLit = 2.5;
    } else if (average > 22.25) {
        numLit = 2;
    } else if (average > 21.75) {
        numLit = 1.5;
    } else if (average > 21.25) {
        numLit = 1;
    } else if (average > 20.75) {
        numLit = 0.5;
    } else {
        allBlink = true;
    }

    // Display battery level upon startup for 15 seconds or when button is pressed or when debug is enabled
    if (startupDisplay && (currentMillis - displayMillis < startupDisplayDuration)) {
        setLEDs(numLit, currentMillis, blinkInterval, allBlink);
    } else if (displayBatteryLevel && (currentMillis - displayMillis < displayDuration)) {
        setLEDs(numLit, currentMillis, blinkInterval, allBlink);
    } else if (allBlink || debugEnabled) {
        setLEDs(numLit, currentMillis, blinkInterval, allBlink);
    } else {
        // Turn off LEDs if not displaying battery level
        for (int i = 0; i < 5; i++) {
            digitalWrite(ledPins[i], LOW);
        }
        displayBatteryLevel = false;
        startupDisplay = false;
    }

    // Serial output
    if (debugEnabled) {
        Serial.print("Voltage: ");
        Serial.print((analogRead(analogPin) * conversionFactor - 0.1));
        Serial.print(" V, Voltage avg: ");
        Serial.print(average);
        Serial.print(" V, StdDev: ");
        Serial.println(stdDev);
    }

    delay(100); // Small delay to stabilize readings
}

void setLEDs(float numLit, unsigned long currentMillis, long interval, bool allBlink) {
    if (allBlink) {
        if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis;
            ledState = !ledState;
            for (int i = 0; i < 5; i++) {
                digitalWrite(ledPins[i], ledState);
            }
        }
    } else {
        int fullLit = floor(numLit);
        bool blinking = (numLit - fullLit) >= 0.5;

        for (int i = 0; i < fullLit; i++) {
            digitalWrite(ledPins[i], HIGH);
        }

        if (blinking && fullLit < 5) {
            if (currentMillis - previousMillis >= interval) {
                previousMillis = currentMillis;
                ledState = !ledState;
                digitalWrite(ledPins[fullLit], ledState);
            }
        } else if (fullLit < 5) {
            digitalWrite(ledPins[fullLit], LOW);
        }

        for (int i = fullLit + 1; i < 5; i++) {
            digitalWrite(ledPins[i], LOW);
        }
    }
}

void enableDebug() {
    debugEnabled = true;
}

void disableDebug() {
    debugEnabled = false;
}

void handleSerialInput() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();

        if (command.equalsIgnoreCase("DEBUG BATTERY LEVEL ON")) {
            enableDebug();
            Serial.println("Battery level debugging enabled.");
        } else if (command.equalsIgnoreCase("DEBUG BATTERY LEVEL OFF")) {
            disableDebug();
            Serial.println("Battery level debugging disabled.");
        }
    }
}
