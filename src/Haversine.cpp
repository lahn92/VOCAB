#include <Arduino.h>
#include "Haversine.h"

HaversineOutput haversine(double lat1, double lon1, double lat2, double lon2) {
  const double R = 6371000;
  double lat1_rad = radians(lat1);
  double lat2_rad = radians(lat2);
  double dLat = radians(lat2 - lat1);
  double dLon = radians(lon2 - lon1);

  double a = sin(dLat / 2) * sin(dLat / 2) +
             sin(dLon / 2) * sin(dLon / 2) * cos(lat1_rad) * cos(lat2_rad);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));
  double distance = R * c;

  double y = sin(dLon) * cos(lat2_rad);
  double x = cos(lat1_rad) * sin(lat2_rad) -
             sin(lat1_rad) * cos(lat2_rad) * cos(dLon);
  double bearing_rad = atan2(y, x);
  double bearing_deg = degrees(bearing_rad);
  bearing_deg += 180;
  if (bearing_deg >= 360) {
    bearing_deg -= 360;
  }

  HaversineOutput result;
  result.distance = distance;
  result.bearing = bearing_deg;
  return result;
}
