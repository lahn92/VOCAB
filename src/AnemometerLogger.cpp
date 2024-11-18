#include "AnemometerLogger.h"

AnemometerLogger::AnemometerLogger(int ledPin, Anemometer& anemometer, unsigned long logInterval)
    : ledPin(ledPin), anemometer(anemometer), logInterval(logInterval), usb("usb"), otg(PB_8, 1), previousMillis(0), lastUSBCheckMillis(0), usbMounted(false) {}

void AnemometerLogger::setup() {
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW); // Turn off LED initially
    while (!Serial);

    if (connectUSB()) {
        currentFileName = createUniqueFileName();
        initializeLogFile();
    }
}

void AnemometerLogger::loop() {
    unsigned long currentMillis = millis();

    // Update anemometer readings continuously
    anemometer.update();

    // Check if the log interval has passed
    if (currentMillis - previousMillis >= logInterval) {
        previousMillis = currentMillis;

        // Calculate the average wind speed over the past 10 minutes
        float averageWindSpeed = anemometer.getAverageWindSpeed(); // Assuming the method calculates over the correct interval

        // Ensure USB is still connected before trying to write
        if (usbMounted && msd.connected()) {
            FILE *f = fopen(currentFileName.c_str(), "a");
            if (f) {
                // Log time and average wind speed
                if (fprintf(f, "%lu;%f\n", currentMillis / 1000, averageWindSpeed) < 0) {
                    Serial.println("Error writing to file");
                } else {
                    Serial.println("Data logged to file: " + currentFileName);
                }
                fclose(f);
            } else {
                Serial.println("Error opening file for appending");
            }
        } else {
            Serial.println("USB flash drive not mounted or disconnected.");
            handleUSBDisconnection();
        }
    }
}

bool AnemometerLogger::isUSBConnected() {
    return msd.connected();
}

bool AnemometerLogger::connectUSB() {
    Serial.println("Connecting to USB device...");
    unsigned long startAttemptTime = millis();
    while (millis() - startAttemptTime < 10000) { // Try for up to 10 seconds
        if (msd.connect()) {
            Serial.println("USB device connected.");
            return true;
        }
    }
    Serial.println("Failed to connect to USB device after multiple attempts.");
    return false;
}

bool AnemometerLogger::mountUSB() {
    Serial.println("Mounting USB device...");
    unsigned long startAttemptTime = millis();
    while (millis() - startAttemptTime < 10000) { // Try for up to 10 seconds
        int err = usb.mount(&msd);
        if (err == 0) {
            Serial.println("USB flash drive mounted successfully.");
            usbMounted = true;
            digitalWrite(ledPin, HIGH); // Turn on LED when mounted
            return true;
        }
    }
    Serial.println("Failed to mount USB flash drive after multiple attempts.");
    return false;
}

void AnemometerLogger::handleUSBDisconnection() {
    Serial.println("USB flash drive disconnected.");
    usb.unmount();  // Unmount the file system
    usbMounted = false;
    digitalWrite(ledPin, LOW); // Turn off LED when disconnected

    while (!msd.connect()) {
        unsigned long reconnectAttemptStart = millis();
        while (millis() - reconnectAttemptStart < 1000) {
            // Wait for 1 second before retrying
        }
        Serial.println("Waiting for USB flash drive to reconnect...");
    }
    if (connectUSB()) {
        initializeLogFile(true); // Append to the existing log file
    }
}

void AnemometerLogger::initializeLogFile(bool append) {
    if (mountUSB()) {
        // Open the existing file if append is true, else create a new file
        FILE *f = fopen(currentFileName.c_str(), append ? "a" : "w+");
        if (f) {
            if (!append) {
                fprintf(f, "Time;Average Wind Speed\n"); // Write header only if creating a new file
                Serial.println("Created new file: " + currentFileName);
            } else {
                Serial.println("Appending to existing file: " + currentFileName);
            }
            fclose(f);
        } else {
            Serial.println("Error opening file for writing");
        }
    }
}

String AnemometerLogger::createUniqueFileName() {
    int maxIndex = getMaxFileIndex();
    maxIndex++;
    String baseName = "/usb/anemometer";
    String extension = ".txt";
    String fileName = baseName + maxIndex + extension;
    Serial.println("Generated unique file name: " + fileName);
    return fileName;
}

int AnemometerLogger::getMaxFileIndex() {
    int maxIndex = 0;
    DIR *dir;
    struct dirent *ent;
    char path[] = "/usb";
    if ((dir = opendir(path)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            String fileName = String(ent->d_name);
            Serial.print("Checking file: ");
            Serial.println(fileName);
            if (fileName.startsWith("anemometer") && fileName.endsWith(".txt")) {
                int startIdx = String("anemometer").length();
                int endIdx = fileName.length() - String(".txt").length();
                String indexStr = fileName.substring(startIdx, endIdx);
                Serial.print("Extracted index string: ");
                Serial.println(indexStr);
                if (indexStr.length() > 0 && indexStr.toInt() > 0) { // Check if indexStr is not empty and a valid number
                    int fileIdx = indexStr.toInt();
                    Serial.print("Parsed file index: ");
                    Serial.println(fileIdx);
                    if (fileIdx > maxIndex) {
                        maxIndex = fileIdx;
                    }
                }
            }
        }
        closedir(dir);
    } else {
        Serial.println("Failed to open directory.");
    }
    Serial.print("Max file index found: ");
    Serial.println(maxIndex);
    return maxIndex;
}
