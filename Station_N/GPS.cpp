// #include "GPSConfig.h"
#include "Config.h"

TinyGPSPlus gps;
SoftwareSerial gpsSerial(GPS_RX_PIN, GPS_TX_PIN);

void setupGPS() {
    gpsSerial.begin(GPS_BAUD);
    Serial.println("GPS Module Initialized");
}

void updateGPS() {
    while (gpsSerial.available() > 0) {
        if (gps.encode(gpsSerial.read())) {
            // New valid GPS data received
            if (gps.location.isUpdated()) {
                // displayGPSInfo();
            }
        }
    }

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

    // if (gps.date.isValid()) {
    //     Serial.print(F("Date: "));
    //     Serial.print(gps.date.month());
    //     Serial.print(F("/"));
    //     Serial.print(gps.date.day());
    //     Serial.print(F("/"));
    //     Serial.println(gps.date.year());
    // }

    // if (gps.time.isValid()) {
    //     Serial.print(F("Time: "));
    //     if (gps.time.hour() < 10) Serial.print(F("0"));
    //     Serial.print(gps.time.hour());
    //     Serial.print(F(":"));
    //     if (gps.time.minute() < 10) Serial.print(F("0"));
    //     Serial.print(gps.time.minute());
    //     Serial.print(F(":"));
    //     if (gps.time.second() < 10) Serial.print(F("0"));
    //     Serial.println(gps.time.second());
    // }

    // Serial.print(F("Satellites: "));
    // Serial.println(gps.satellites.value());
    // Serial.print(F("Altitude: "));
    // if (gps.altitude.isValid())
    //     Serial.println(gps.altitude.meters());
    // else
    //     Serial.println(F("Not Available"));

    Serial.println();
}