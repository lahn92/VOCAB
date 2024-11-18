#include "StateMachine.h"
#include "MotorControl.h"
#include "GNSSHandler.h"
#include "WindVaneSensor.h"
#include "Haversine.h"
#include "WaterSensor.h"
#include "SatelliteModem.h"

State currentState = RC_STATE;
POS_SubState posSubState = DRIFTING;
HaversineOutput result;
SFE_UBLOX_GNSS myGNSS;
double relPosHeadingDegrees = 0;
unsigned long lastSignalTime = 0;
unsigned long posStateEnterTime = 0;
unsigned long lastLARSMessageTime = 0; // Add a timer for LARS state message

const unsigned long LARSMessageInterval = 30000; // 2 minutes in milliseconds

double savedLat = 0;
double savedLon = 0;
bool hasSaved = false;
unsigned long saveTime = 0;
bool waitForStabilization = false;

bool debugThrust = false;
bool debugGNSS = false;
bool debugState = false;
bool debugWater = false;

extern Anemometer anemometer; // Declare anemometer as external
extern bool debugAnemometer;  // Declare debugAnemometer as external
extern bool debugVindvane;    // Declare debugVindvane as external

extern WaterSensor waterSensor; // Declare waterSensor as external
extern SatelliteModem satelliteModem; // Declare satelliteModem as external

void printCurrentState() {
    switch (currentState) {
        case RC_STATE:
            Serial.println("Current State: RC");
            break;
        case POS_STATE:
            Serial.println("Current State: POS");
            break;
        case LARS_STATE:
            Serial.println("Current State: LARS");
            break;
    }
    printPosSubState();
}

void printPosSubState() {
    switch (posSubState) {
        case DRIFTING:
            Serial.println("POS SubState: DRIFTING");
            break;
        case ALIGNMENT:
            Serial.println("POS SubState: ALIGNMENT");
            break;
        case RETURN_HOME:
            Serial.println("POS SubState: RETURN_HOME");
            break;
    }
}

void checkStateTransition() {
    unsigned long duration = pulseIn(4, HIGH);
    unsigned long currentMillis = millis();

    if (duration >= 800 && duration <= 1000 && currentState != RC_STATE) {
        currentState = RC_STATE;
        posSubState = DRIFTING;
        printCurrentState();
    } 
    else if (duration >= 1800 && duration <= 2100 && currentState != POS_STATE) {
        if (currentState == RC_STATE && myGNSS.getFixType()) {
            saveCoordinates();
        }
        currentState = POS_STATE;
        posStateEnterTime = millis();
        printCurrentState();
    }

    if (currentState == RC_STATE && duration == 0 && millis() - lastSignalTime > 1000) {
        setThrust(0, 0);
    }

    if (currentState == POS_STATE && millis() - posStateEnterTime >= 30000) {
        currentState = LARS_STATE;
        posSubState = DRIFTING;
        lastLARSMessageTime = millis(); // Reset the LARS message timer
        
        double latestLat = myGNSS.getLatitude() / 10000000.0;
        double latestLon = myGNSS.getLongitude() / 10000000.0;

        Serial.println("About to send message...");

        // Placeholder for future action
        printCurrentState();
    }

    if (currentState == POS_STATE && waterSensor.check(debugWater)) {
        satelliteModem.sendWaterDetectedMessage();
        currentState = LARS_STATE;
        posSubState = DRIFTING;
        lastLARSMessageTime = millis(); // Reset the LARS message timer
        printCurrentState();
    }

    if (debugState) {
        Serial.print("Debug - State: ");
        Serial.print(currentState);
        Serial.print(", SubState: ");
        Serial.println(posSubState);
    }
}

void updateStateMachine(unsigned long currentMillis) {
    switch (currentState) {
        case RC_STATE:
            driveRobot();
            if (!isControllerConnected()) {
                currentState = POS_STATE;
                posStateEnterTime = millis();
                saveCoordinates(); // Save coordinates before transitioning to POS_STATE
                printCurrentState();
            }
            break;
        case POS_STATE:
            readWindVaneSensor(currentMillis, debugVindvane);  // Pass debugVindvane as the second argument
            myGNSS.checkUblox();
            myGNSS.checkCallbacks();
            printLatLonSpeed();
            updatePosStateMachine(result);

            if (waterSensor.check(debugWater)) {
                satelliteModem.sendWaterDetectedMessage();
                currentState = LARS_STATE;
                posSubState = DRIFTING;
                lastLARSMessageTime = millis(); // Reset the LARS message timer
                printCurrentState();
            }
            break;
        case LARS_STATE:
            // Check if 15 minutes have passed since the last LARS state message
            if (currentMillis - lastLARSMessageTime >= LARSMessageInterval) {
                satelliteModem.sendLARSStateMessage();
                lastLARSMessageTime = currentMillis; // Reset the timer
            }
            break;
    }
}

void updatePosStateMachine(HaversineOutput result) {
    switch (posSubState) {
        case DRIFTING:
            setThrust(0, 0);
            if (result.distance > 10) {
                posSubState = ALIGNMENT;
            }
            break;
        case ALIGNMENT:
            if (abs(result.bearing - relPosHeadingDegrees) <= 90) {
                posSubState = RETURN_HOME;
                setThrust(0, 0);
                Serial.println("0,0");
            } else if (relPosHeadingDegrees < result.bearing) {
                setThrust(26, 26);
                Serial.println("26,26");
            } else {
                setThrust(-26, -26);
                Serial.println("-26,-26");
            }
            break;
        case RETURN_HOME:
            if (abs(result.bearing - relPosHeadingDegrees) <= 15) {
                setThrust(25, -25);
                Serial.println("25,-25");
            } else if (relPosHeadingDegrees < result.bearing) {
                setThrust(50, -25);
                Serial.println("50,-35");
            } else {
                setThrust(25, -50);
                Serial.println("25,-50");
            }
            if (result.distance <= 4) {
                posSubState = DRIFTING;
            }
            if (abs(result.bearing - relPosHeadingDegrees > 90)) {
            posSubState = ALIGNMENT;  
            }
            break;
    }
    printPosSubState();
}

void printCurrentStatePeriodically(unsigned long currentMillis) {
    static unsigned long lastMillisPrint = 0;
    if (currentMillis - lastMillisPrint >= 2000) {
        printCurrentState();
        lastMillisPrint = currentMillis;
    }

    if (debugState) {
        Serial.print("Debug - Periodic State: ");
        Serial.print(currentState);
        Serial.print(", SubState: ");
        Serial.println(posSubState);
    }
}

void checkSerialCommands() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        if (command == "RC" && currentState != RC_STATE) {
            currentState = RC_STATE;
            posSubState = DRIFTING;  // Ensure posSubState is set to DRIFTING
        } else if (command == "POS" && currentState != POS_STATE) {
            if (currentState == RC_STATE && myGNSS.getFixType()) {
                saveCoordinates(); // Save coordinates before transitioning to POS_STATE
            }
            currentState = POS_STATE;
            posStateEnterTime = millis();
        } else if (command == "LARS" && currentState != LARS_STATE) {
            currentState = LARS_STATE;
            posSubState = DRIFTING;
            lastLARSMessageTime = millis(); // Reset the LARS message timer
        } else if (command == "DEBUG THRUST ON") {
            debugThrust = true;
            Serial.println("Thrust debug mode enabled");
        } else if (command == "DEBUG THRUST OFF") {
            debugThrust = false;
            Serial.println("Thrust debug mode disabled");
        } else if (command == "DEBUG GNSS ON") {
            debugGNSS = true;
            Serial.println("GNSS debug mode enabled");
        } else if (command == "DEBUG GNSS OFF") {
            debugGNSS = false;
            Serial.println("GNSS debug mode disabled");
        } else if (command == "DEBUG STATE ON") {
            debugState = true;
            Serial.println("State debug mode enabled");
        } else if (command == "DEBUG STATE OFF") {
            debugState = false;
            Serial.println("State debug mode disabled");
        } else if (command == "DEBUG WATER ON") {
            debugWater = true;
            Serial.println("Water sensor debug mode enabled");
        } else if (command == "DEBUG WATER OFF") {
            debugWater = false;
            Serial.println("Water sensor debug mode disabled");
        } else if (command == "DEBUG ANEMOMETER ON") {
            debugAnemometer = true;
            anemometer.setDebug(true);
            Serial.println("Anemometer debug mode enabled");
        } else if (command == "DEBUG ANEMOMETER OFF") {
            debugAnemometer = false;
            anemometer.setDebug(false);
            Serial.println("Anemometer debug mode disabled");
        } else if (command == "DEBUG VINDVANE ON") {
            debugVindvane = true;
            Serial.println("Wind vane sensor debug mode enabled");
        } else if (command == "DEBUG VINDVANE OFF") {
            debugVindvane = false;
            Serial.println("Wind vane sensor debug mode disabled");
        } else if (command.startsWith("(") && command.endsWith(")")) {
            command = command.substring(1, command.length() - 1);
            int firstComma = command.indexOf(',');
            int secondComma = command.lastIndexOf(',');
            if (firstComma > 0 && secondComma > firstComma) {
                result.distance = command.substring(0, firstComma).toDouble();
                result.bearing = command.substring(firstComma + 1, secondComma).toDouble();
                relPosHeadingDegrees = command.substring(secondComma + 1).toDouble();
                Serial.print("Distance: ");
                Serial.print(result.distance);
                Serial.print(", Bearing: ");
                Serial.print(result.bearing);
                Serial.print(", Heading: ");
                Serial.println(relPosHeadingDegrees);
            } else {
                Serial.println("Invalid command format. Use (D,B,R)");
            }
        } else {
            Serial.println("Invalid command");
        }
        printCurrentState();
    }
}
