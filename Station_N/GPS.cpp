#include "Config.h"

void setupGPS() {
    gpsSerial.begin(GPS_BAUD);
    Serial.println("GPS Module Initialized");
}

void updateGPS() {
    // Check if GPS is working after 5 seconds
    if (millis() > 5000 && gps.charsProcessed() < 10) {
        Serial.println(F("No GPS detected"));
    }
}

bool isGPSValid() {
    return gps.location.isValid();
}

float getLatitude() {
    if (gps.location.isValid()) {
        return gps.location.lat();
    }
    return 0.0;
}

float getLongitude() {
    if (gps.location.isValid()) {
        return gps.location.lng();
    }
    return 0.0;
}

void displayGPSInfo() {
    if (gps.location.isValid()) {
        Serial.print(F("Location: "));
        Serial.print(gps.location.lat(), 6);
        Serial.print(F(","));
        Serial.println(gps.location.lng(), 6);
    } else {
        Serial.println(F("Location: Not Available"));
    }

    if (gps.date.isValid()) {
        Serial.print(F("Date: "));
        Serial.print(gps.date.month());
        Serial.print(F("/"));
        Serial.print(gps.date.day());
        Serial.print(F("/"));
        Serial.println(gps.date.year());
    }

    Serial.println();
}