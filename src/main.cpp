#include <Arduino.h>
#include "mbed.h"
#include "rtos.h"
#include "platform/Callback.h"
#include "StateMachine.h"
#include "MotorControl.h"
#include "GNSSHandler.h"
#include "WindVaneSensor.h"
#include "Haversine.h"
#include "WaterSensor.h"
#include "Anemometer.h"
#include "SatelliteModem.h"
#include "BatteryLevel.h"

using namespace rtos;
using namespace mbed;

// Pin definitions
const int ledPin = 39; // LED pin
const int anemometerPin = A3; // Anemometer pin

// Objects
WaterSensor waterSensor(A0, A1); // Instantiate waterSensor here
Anemometer anemometer(anemometerPin); // Anemometer on A3
SatelliteModem satelliteModem; // Instantiate SatelliteModem here

unsigned long lastWindVaneUpdate = 0;
const unsigned long windVaneInterval = 2000;
bool serialOutputEnabledWindVane = true;
bool debugAnemometer = false;  // Debug mode for anemometer
bool debugVindvane = false;    // Debug mode for wind vane

Thread anemometerThread;
Thread batteryThread;


void AnemometerTask() {
    while (true) {
        anemometer.update();  // Update the anemometer readings
        ThisThread::sleep_for(10);  // Adjust the interval as needed
    }
}

void BatteryTask() {
    while (true) {
        loopBatteryLevel();
        ThisThread::sleep_for(80);  // Adjust the interval as needed
    }
}

void setup() {
    //Serial.begin(115200);  // Ensure this is at the beginning of the setup
    // while (!Serial && millis() < 5000);       // Wait for the Serial to initialize
    
    Serial.println("Initializing system...");
    Serial.println("Initializing Satellite Modem...");
    delay(1000);
    satelliteModem.setup();  // Initialize the Satellite Modem
    setupMotorControl();
    setupGNSS();
    
   
    setupBatteryLevel();
    anemometer.setup();
    printCurrentState();
    
    // Start the anemometer thread
    anemometerThread.start(callback(AnemometerTask));
    batteryThread.start(callback(BatteryTask));

    Serial.println("System initialization complete.");
}

void loop() {
    unsigned long currentMillis = millis();

    checkSerialCommands();
    checkStateTransition();
    updateStateMachine(currentMillis);
    printCurrentStatePeriodically(currentMillis);

    // Always check for water and send a message if detected
    if (currentState == POS_STATE && waterSensor.check(debugWater)) {
        satelliteModem.sendWaterDetectedMessage();
    }

    // Read Wind Vane Sensor
    if (currentMillis - lastWindVaneUpdate >= windVaneInterval) {
        readWindVaneSensor(currentMillis, debugVindvane);
        lastWindVaneUpdate = currentMillis;
    }
}
