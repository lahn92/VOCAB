#ifndef HAVERSINE_H
#define HAVERSINE_H

#include <Arduino.h>

struct HaversineOutput {
  double distance;
  double bearing;
};

HaversineOutput haversine(double lat1, double lon1, double lat2, double lon2);

#endif
