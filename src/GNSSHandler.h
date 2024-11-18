#ifndef GNSSHANDLER_H
#define GNSSHANDLER_H

#include <Arduino.h>
#include <SparkFun_u-blox_GNSS_v3.h>

void setupGNSS();
void printRELPOSNEDdata(UBX_NAV_RELPOSNED_data_t *ubxDataStruct);
void saveCoordinates();
void printLatLonSpeed();

#endif
