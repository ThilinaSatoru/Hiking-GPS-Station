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

// Map to store connected stations
std::map<uint32_t, StationInfo> connectedStations;
unsigned long lastConnectionCheck = 0;

// Function to log station status
void logStationStatus(bool isConnected, int stationNo, String nodeId) {
    DynamicJsonDocument statusDoc(256);
    statusDoc["station"] = stationNo;
    statusDoc["online"] = isConnected;
    statusDoc["nodeId"] = nodeId;
    statusDoc["type"] = "status";

    String statusMsg;
    serializeJson(statusDoc, statusMsg);
    Serial.println(statusMsg);
}

// Function to check stations' connection status
void checkStationsConnection() {
    unsigned long currentTime = millis();

    for (auto& station : connectedStations) {
        bool wasOnline = station.second.isOnline;
        station.second.isOnline = (currentTime - station.second.lastSeen) < NODE_TIMEOUT;

        // Log only when status changes
        if (wasOnline != station.second.isOnline) {
            logStationStatus(station.second.isOnline,
                           station.second.stationNo,
                           station.second.nodeId);
        }
    }
}

// Function to update station's last seen time
void updateStationActivity(uint32_t nodeId, int stationNo, String nodeIdStr) {
    unsigned long currentTime = millis();

    if (connectedStations.find(nodeId) == connectedStations.end()) {
        // New station connected
        StationInfo newStation = {
            stationNo,
            nodeIdStr,
            currentTime,
            true
        };
        connectedStations[nodeId] = newStation;
        logStationStatus(true, stationNo, nodeIdStr);
    } else {
        // Update existing station
        connectedStations[nodeId].lastSeen = currentTime;
        if (!connectedStations[nodeId].isOnline) {
            connectedStations[nodeId].isOnline = true;
            logStationStatus(true, stationNo, nodeIdStr);
        }
    }
}

// Function to send confirmation back to the sender
void sendConfirmation(uint32_t to, int stationNo, String nodeId) {
    DynamicJsonDocument confirmDoc(256);
    confirmDoc["type"] = "confirmation";
    confirmDoc["station"] = stationNo;
    confirmDoc["nodeId"] = nodeId;

    String confirmMsg;
    serializeJson(confirmDoc, confirmMsg);
    mesh.sendSingle(to, confirmMsg);
}

// Callback for new connections
void newConnectionCallback(uint32_t nodeId) {
    DynamicJsonDocument doc(128);
    doc["type"] = "request";
    doc["requestType"] = "identify";

    String msg;
    serializeJson(doc, msg);
    mesh.sendSingle(nodeId, msg);
}

// Callback for dropped connections
void droppedConnectionCallback(uint32_t nodeId) {
    if (connectedStations.find(nodeId) != connectedStations.end()) {
        logStationStatus(false,
                        connectedStations[nodeId].stationNo,
                        connectedStations[nodeId].nodeId);
        connectedStations[nodeId].isOnline = false;
    }
}

// Function to handle received messages
void receivedCallback(uint32_t from, String &msg) {
    DynamicJsonDocument data(1024);
    DeserializationError error = deserializeJson(data, msg);

    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
    }

    String messageType = data["type"];

    if (messageType == "data") {
        int stationNo = data["station"];
        String nodeId = data["nodeId"];

        // Update station's activity
        updateStationActivity(from, stationNo, nodeId);

        // Forward the data to serial
        String jsonData;
        serializeJson(data, jsonData);
        Serial.println(jsonData);

        // Send confirmation back to the sender
        sendConfirmation(from, stationNo, nodeId);
    }
    else if (messageType == "identify") {
        // Handle identification response
        int stationNo = data["station"];
        String nodeId = data["nodeId"];
        updateStationActivity(from, stationNo, nodeId);
    }
}

void setup() {
    Serial.begin(115200);

    mesh.setDebugMsgTypes(ERROR | STARTUP);  // Reduced debug messages
    mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onDroppedConnection(&droppedConnectionCallback);
    mesh.setRoot(true);
    mesh.setContainsRoot(true);

    Serial.println("{\"type\":\"status\",\"message\":\"Mesh network initialized as root\"}");
}

void loop() {
    mesh.update();

    // Check stations connection status periodically
    unsigned long currentTime = millis();
    if (currentTime - lastConnectionCheck >= CONNECTION_CHECK_INTERVAL) {
        checkStationsConnection();
        lastConnectionCheck = currentTime;
    }
}