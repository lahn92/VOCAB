#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <Arduino.h>
#include <SparkFun_u-blox_GNSS_v3.h>
#include "Haversine.h"
#include "Anemometer.h" // Add this line to include the Anemometer class
#include "WaterSensor.h" // Add this line to include the WaterSensor class

enum State {
    RC_STATE,
    POS_STATE,
    LARS_STATE
};

enum POS_SubState {
    DRIFTING,
    ALIGNMENT,
    RETURN_HOME
};

void printCurrentState();
void printPosSubState();
void updatePosStateMachine(HaversineOutput result);
void checkStateTransition();
void updateStateMachine(unsigned long currentMillis);
void printCurrentStatePeriodically(unsigned long currentMillis);
void checkSerialCommands();

extern State currentState;
extern POS_SubState posSubState;
extern HaversineOutput result;
extern SFE_UBLOX_GNSS myGNSS;
extern double relPosHeadingDegrees;
extern unsigned long lastSignalTime;
extern unsigned long posStateEnterTime;

extern double savedLat;
extern double savedLon;
extern bool hasSaved;
extern unsigned long saveTime;
extern bool waitForStabilization;

extern bool debugThrust;
extern bool debugGNSS;
extern bool debugState;
extern bool debugWater;

extern Anemometer anemometer; // Declare anemometer as external
extern WaterSensor waterSensor; // Declare waterSensor as external

#endif
