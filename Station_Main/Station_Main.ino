#include <painlessMesh.h>
#include <ArduinoJson.h>
#include <map>

#define MESH_PREFIX     "HIKING"
#define MESH_PASSWORD   "meshPassword"
#define MESH_PORT       5555

#define CONNECTION_CHECK_INTERVAL 30000  // Check connection every 30 seconds
#define NODE_TIMEOUT 60000              // Consider node offline after 60 seconds

painlessMesh mesh;

// Structure to store station information
struct StationInfo {
    int stationNo;
    String nodeId;
    unsigned long lastSeen;
    bool isOnline;
};



void setup() {
    Serial.begin(115200);

    Serial.println("{\"type\":\"status\",\"message\":\"Mesh network initialized as root\"}");
}

void loop() {
    mesh.update();
}