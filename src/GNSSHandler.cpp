#include <Arduino.h>
#include "GNSSHandler.h"
#include "StateMachine.h"
#include "Haversine.h"

void setupGNSS() {
  Wire.setClock(400000);
  while (myGNSS.begin() == false) {
    Serial.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Retrying..."));
    delay(1000);
  }

  Serial.println(F("u-blox GNSS module connected. Waiting for valid fix..."));
  while (!myGNSS.getFixType()) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(F("GNSS fix acquired!"));

  myGNSS.setAutoRELPOSNEDcallbackPtr(&printRELPOSNEDdata);
}

void saveCoordinates() {
  savedLat = myGNSS.getLatitude() / 10000000.0;
  savedLon = myGNSS.getLongitude() / 10000000.0;
  hasSaved = true;
  saveTime = millis();
  waitForStabilization = true;
  Serial.print("Coordinates saved on entering POS_STATE. Lat: ");
  Serial.print(savedLat, 9);
  Serial.print(", Lon: ");
  Serial.println(savedLon, 9);
}

void printRELPOSNEDdata(UBX_NAV_RELPOSNED_data_t *ubxDataStruct) {
  Serial.println();
  Serial.print("relPosLength (m): ");
  Serial.println(((double)ubxDataStruct->relPosLength / 100) + ((double)ubxDataStruct->relPosHPLength / 10000), 4);
  Serial.print("relPosHeading (Deg): ");
  Serial.println((double)ubxDataStruct->relPosHeading / 100000);

  relPosHeadingDegrees = (double)ubxDataStruct->relPosHeading / 100000;
}

void printLatLonSpeed() {
  int32_t latitude = myGNSS.getHighResLatitude();
  int8_t latitudeHp = myGNSS.getHighResLatitudeHp();
  int32_t longitude = myGNSS.getHighResLongitude();
  int8_t longitudeHp = myGNSS.getHighResLongitudeHp();

  double d_lat = ((double)latitude) / 10000000.0;
  d_lat += ((double)latitudeHp) / 1000000000.0;
  double d_lon = ((double)longitude) / 10000000.0;
  d_lon += ((double)longitudeHp) / 1000000000.0;

  Serial.println();
  Serial.print("Lat (deg): ");
  Serial.print(d_lat, 9);
  Serial.print(", Lon (deg): ");
  Serial.print(d_lon, 9);

  if (currentState == POS_STATE) {
    result = haversine(savedLat, savedLon, d_lat, d_lon);
    Serial.print("Distance to saved point: ");
    Serial.print(result.distance);
    Serial.println(" meters");
    Serial.print("Bearing to saved point: ");
    Serial.print(result.bearing);
    Serial.println(" degrees");
  }

  if (debugGNSS) {
    Serial.print("Debug - Lat: ");
    Serial.print(d_lat, 9);
    Serial.print(", Lon: ");
    Serial.print(d_lon, 9);
    Serial.print(", Speed: ");
    Serial.println(myGNSS.getGroundSpeed());
  }
}
