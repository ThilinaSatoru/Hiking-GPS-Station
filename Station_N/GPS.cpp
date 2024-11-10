#include "Config.h"

void setupGPS() {
    gpsSerial.begin(GPS_BAUD);
    Serial.println("GPS Module Initialized");
}
