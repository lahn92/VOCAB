#ifndef ANEMOMETERLOGGER_H
#define ANEMOMETERLOGGER_H

#include <Arduino.h>
#include <Arduino_USBHostMbed5.h>
#include <DigitalOut.h>
#include <FATFileSystem.h>
#include "Anemometer.h"

class AnemometerLogger {
public:
    AnemometerLogger(int ledPin, Anemometer& anemometer, unsigned long logInterval = 60000); // Default log interval is 1 minute
    void setup();
    void loop();
    bool isUSBConnected(); // Public method to check USB connection status
    void handleUSBDisconnection(); // Public method to handle USB disconnection

private:
    int ledPin;
    Anemometer& anemometer;
    unsigned long logInterval;
    unsigned long previousMillis;
    unsigned long lastUSBCheckMillis;
    USBHostMSD msd;
    mbed::FATFileSystem usb;
    mbed::DigitalOut otg;
    bool usbMounted;
    String currentFileName;

    bool connectUSB();
    bool mountUSB();
    void initializeLogFile(bool append = false); // Add append parameter
    String createUniqueFileName();
    int getMaxFileIndex();
};

#endif // ANEMOMETERLOGGER_H
