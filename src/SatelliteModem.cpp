#include "SatelliteModem.h"
#include <Arduino.h>

#define IridiumSerial Serial2
#define DIAGNOSTICS false // Change this to see diagnostics

// Declare the IridiumSBD object
IridiumSBD modem(IridiumSerial);

SatelliteModem::SatelliteModem() : modem(IridiumSerial), signalQuality(-1), waterDetectedSent(false) {}

void SatelliteModem::setup() {
    int err;

    // Start the console serial port
    //Serial.begin(115200);
    // while (!Serial);

    // Start the serial port connected to the satellite modem
    IridiumSerial.begin(19200);

    // Begin satellite modem operation
    Serial.println("Starting modem...");
    err = modem.begin();
    if (err != ISBD_SUCCESS) {
        Serial.print("Begin failed: error ");
        Serial.println(err);
        if (err == ISBD_NO_MODEM_DETECTED)
            Serial.println("No modem detected: check wiring.");
        return;
    }

    // Example: Print the firmware revision
    char version[12];
    err = modem.getFirmwareVersion(version, sizeof(version));
    if (err != ISBD_SUCCESS) {
        Serial.print("FirmwareVersion failed: error ");
        Serial.println(err);
        return;
    }
    Serial.print("Firmware Version is ");
    Serial.print(version);
    Serial.println(".");

    // Example: Test the signal quality.
    // This returns a number between 0 and 5.
    // 2 or better is preferred.
    err = modem.getSignalQuality(signalQuality);
    if (err != ISBD_SUCCESS) {
        Serial.print("SignalQuality failed: error ");
        Serial.println(err);
        return;
    }

    Serial.print("On a scale of 0 to 5, signal quality is currently ");
    Serial.print(signalQuality);
    Serial.println(".");
}

void SatelliteModem::sendWaterDetectedMessage() {
    if (waterDetectedSent) {
        Serial.println("Water detected message already sent.");
        return; // Do not send the message again
    }

    int err;
    
    // Ensure we have a decent signal quality before sending
    err = modem.getSignalQuality(signalQuality);
    if (err != ISBD_SUCCESS || signalQuality < 2) {
        Serial.print("Poor signal quality, cannot send message. Error: ");
        Serial.println(err);
        return;
    }

    Serial.print("Trying to send the message. This might take several minutes.\r\n");
    err = modem.sendSBDText("Water detected");
    if (err != ISBD_SUCCESS) {
        Serial.print("sendSBDText failed: error ");
        Serial.println(err);
        if (err == ISBD_SENDRECEIVE_TIMEOUT)
            Serial.println("Try again with a better view of the sky.");
    } else {
        Serial.println("Hey, it worked!");
        waterDetectedSent = true; // Set the flag to indicate the message has been sent
    }
}

void SatelliteModem::sendLARSStateMessage() {
    int err;
    
    // Ensure we have a decent signal quality before sending
    err = modem.getSignalQuality(signalQuality);
    if (err != ISBD_SUCCESS || signalQuality < 2) {
        Serial.print("Poor signal quality, cannot send message. Error: ");
        Serial.println(err);
        return;
    }

    Serial.print("Trying to send the LARS state message. This might take several minutes.\r\n");
    err = modem.sendSBDText("VOCAB IN LARS STATE, MISSION IS COMPLETE");
    if (err != ISBD_SUCCESS) {
        Serial.print("sendSBDText failed: error ");
        Serial.println(err);
        if (err == ISBD_SENDRECEIVE_TIMEOUT)
            Serial.println("Try again with a better view of the sky.");
    } else {
        Serial.println("LARS state message sent successfully!");
    }
}

void SatelliteModem::resetWaterDetectionFlag() {
    waterDetectedSent = false;
}

#if DIAGNOSTICS
void ISBDConsoleCallback(IridiumSBD *device, char c) {
    Serial.write(c);
}

void ISBDDiagsCallback(IridiumSBD *device, char c) {
    Serial.write(c);
}
#endif
