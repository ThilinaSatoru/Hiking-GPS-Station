// #include "MeshConfig.h"
// #include "ButtonConfig.h"
// #include "DisplayConfig.h"
// #include "GPSConfig.h"
#include "Config.h"

void setup() {
    Serial.begin(115200);
    
    setupDisplay();
    setupGPS();
    setupButtons();
    setupMesh();
    
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("System Ready!");
    display.display();
    
    delay(2000);
}

void loop() {
    mesh.update();
    handleButtons();
    updateGPS();
    updateDisplay();
    
    delay(50);
}