#include "StateMachine.h"
#include "MotorControl.h"
#include "GNSSHandler.h"
#include "WindVaneSensor.h"
#include "Haversine.h"

State currentState = RC_STATE;
POS_SubState posSubState = DRIFTING;
HaversineOutput result;
SFE_UBLOX_GNSS myGNSS;
double relPosHeadingDegrees = 0;
unsigned long lastSignalTime = 0;
unsigned long posStateEnterTime = 0;

double savedLat = 0.0;
double savedLon = 0.0;
bool hasSaved = false;
unsigned long saveTime = 0;
bool waitForStabilization = false;

bool debugThrust = false;
bool debugGNSS = false;
bool debugState = false;
bool debugWater = false;
bool debugAnemometer = false;  // Initialize debugAnemometer

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
    printCurrentState();
  } else if (duration >= 1800 && duration <= 2100 && currentState != POS_STATE) {
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
  // ****************************************************************************************** HVOR LANGT TID DET TAGER AT GÅ FRA POS TIL STATE *************************************************************************************
  if (currentState == POS_STATE && millis() - posStateEnterTime >= 15000) {
    currentState = LARS_STATE;
    posSubState = DRIFTING;
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
      break;
    case POS_STATE:
      readWindVaneSensor(currentMillis);
      myGNSS.checkUblox();
      myGNSS.checkCallbacks();
      printLatLonSpeed();
      updatePosStateMachine(result);
      break;
    case LARS_STATE:
      // Add behavior for LARS state if required
      break;
  }
}

void updatePosStateMachine(HaversineOutput result) {
  switch (posSubState) {
    case DRIFTING:
      setThrust(0, 0);
      if (result.distance > 15) {
        posSubState = ALIGNMENT;
      }
      break;
    case ALIGNMENT:
      if (abs(result.bearing - relPosHeadingDegrees) <= 10) {
        posSubState = RETURN_HOME;
        setThrust(0, 0);
      } else if (relPosHeadingDegrees < result.bearing) {
        setThrust(10, 0);
      } else {
        setThrust(0, 10);
      }
      break;
    case RETURN_HOME:
      if (abs(result.bearing - relPosHeadingDegrees) <= 10) {
        setThrust(10, 10);
      } else if (relPosHeadingDegrees < result.bearing) {
        setThrust(15, 10);
      } else {
        setThrust(10, 15);
      }
      if (result.distance <= 2) {
        posSubState = DRIFTING;
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
    } else if (command == "POS" && currentState != POS_STATE) {
      currentState = POS_STATE;
      posStateEnterTime = millis();
    } else if (command == "LARS" && currentState != LARS_STATE) {
      currentState = LARS_STATE;
      posSubState = DRIFTING;
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
      Serial.println("Anemometer debug mode enabled");
    } else if (command == "DEBUG ANEMOMETER OFF") {
      debugAnemometer = false;
      Serial.println("Anemometer debug mode disabled");
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
