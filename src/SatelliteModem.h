#ifndef SATELLITEMODEM_H
#define SATELLITEMODEM_H

#include <IridiumSBD.h>

class SatelliteModem {
public:
    SatelliteModem();
    void setup();
    void sendWaterDetectedMessage();
    void sendLARSStateMessage(); // Add method to send LARS state message
    void resetWaterDetectionFlag(); // Add a method to reset the flag

private:
    IridiumSBD modem;
    int signalQuality;
    bool canSendMessage();
    bool waterDetectedSent; // Add a flag to indicate if the water detection message has been sent
};

#endif
