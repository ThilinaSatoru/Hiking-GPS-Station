#include <painlessMesh.h>
#include <ArduinoJson.h>
// #include <ESP8266WiFi.h>
#include <list>

#define MESH_PREFIX     "HIKING"
#define MESH_PASSWORD   "meshPassword"
#define MESH_PORT       5555

#define CONNECTION_CHECK_INTERVAL 40000  // Check connection every 30 seconds
#define NODE_TIMEOUT 60000              // Consider node offline after 60 seconds
#define YIELD_INTERVAL 50               // Yield every 50ms in long operations

painlessMesh mesh;

// Structure to store station information
struct StationInfo {
    int stationNo;
    String nodeId;
    unsigned long lastSeen;
    bool isOnline;
};

// Using list instead of map to reduce memory usage for ESP8266
std::list<std::pair<uint32_t, StationInfo>> connectedStations;
unsigned long lastConnectionCheck = 0;
unsigned long lastYield = 0;

// Helper function to manage yielding
void safeYield() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastYield >= YIELD_INTERVAL) {
        yield();
        lastYield = currentMillis;
    }
}

// Helper function to find station in list
std::list<std::pair<uint32_t, StationInfo>>::iterator findStation(uint32_t nodeId) {
    unsigned int count = 0;
    auto it = connectedStations.begin();
    while (it != connectedStations.end()) {
        if (count++ % 5 == 0) safeYield(); // Yield every 5 iterations
        if (it->first == nodeId) return it;
        ++it;
    }
    return connectedStations.end();
}

// Function to log station status
void logStationStatus(bool isConnected, int stationNo, String nodeId) {
    StaticJsonDocument<256> statusDoc;  // Using StaticJsonDocument to save memory
    statusDoc["station"] = stationNo;
    statusDoc["online"] = isConnected;
    statusDoc["nodeId"] = nodeId;
    statusDoc["type"] = "status";

    String statusMsg;
    serializeJson(statusDoc, statusMsg);
    Serial.println(statusMsg);
    yield(); // Yield after serial operation
}

// Function to check stations' connection status
void checkStationsConnection() {
    unsigned long currentTime = millis();
    
    for (auto& station : connectedStations) {
        safeYield(); // Yield for each station check
        
        bool wasOnline = station.second.isOnline;
        station.second.isOnline = (currentTime - station.second.lastSeen) < NODE_TIMEOUT;
        
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
    auto stationIt = findStation(nodeId);
    
    if (stationIt == connectedStations.end()) {
        // New station connected
        StationInfo newStation = {
            stationNo,
            nodeIdStr,
            currentTime,
            true
        };
        connectedStations.push_back({nodeId, newStation});
        yield(); // Yield after memory allocation
        logStationStatus(true, stationNo, nodeIdStr);
    } else {
        // Update existing station
        stationIt->second.lastSeen = currentTime;
        if (!stationIt->second.isOnline) {
            stationIt->second.isOnline = true;
            logStationStatus(true, stationNo, nodeIdStr);
        }
    }
}

// Function to send confirmation back to the sender
void sendConfirmation(uint32_t to, int stationNo, String nodeId) {
    unsigned long waitStart = millis();
    
    while (!Serial.available()) {
        mesh.update();  // Keep mesh network alive while waiting
        delay(10);     // Built-in yield
        
        // Timeout after 5 seconds of waiting
        if (millis() - waitStart > 5000) {
            Serial.println(F("{\"type\":\"error\",\"message\":\"Serial response timeout\"}"));
            return;
        }
    }
    
    String response = Serial.readStringUntil('\n');
    yield(); // Yield after serial read

    if (response == "true") {
        StaticJsonDocument<256> confirmDoc;
        confirmDoc["type"] = "confirmation";
        confirmDoc["station"] = stationNo;
        confirmDoc["nodeId"] = nodeId;

        String confirmMsg;
        serializeJson(confirmDoc, confirmMsg);
        mesh.sendSingle(to, confirmMsg);
        yield(); // Yield after sending message
    }
}

// Callback for new connections
void newConnectionCallback(uint32_t nodeId) {
    StaticJsonDocument<128> doc;
    doc["type"] = "request";
    doc["requestType"] = "identify";
    
    String msg;
    serializeJson(doc, msg);
    mesh.sendSingle(nodeId, msg);
    yield(); // Yield after sending message
}

// Callback for dropped connections
void droppedConnectionCallback(uint32_t nodeId) {
    auto stationIt = findStation(nodeId);
    if (stationIt != connectedStations.end()) {
        logStationStatus(false, 
                        stationIt->second.stationNo, 
                        stationIt->second.nodeId);
        stationIt->second.isOnline = false;
    }
    yield(); // Yield after connection drop handling
}

// Function to handle received messages
void receivedCallback(uint32_t from, String &msg) {
    StaticJsonDocument<512> data;  // Reduced buffer size for ESP8266
    DeserializationError error = deserializeJson(data, msg);

    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        yield(); // Yield after error handling
        return;
    }

    const char* messageType = data["type"];
    yield(); // Yield after JSON parsing
    
    if (strcmp(messageType, "data") == 0) {
        int stationNo = data["station"];
        const char* nodeId = data["nodeId"];

        updateStationActivity(from, stationNo, nodeId);

        String jsonData;
        serializeJson(data, jsonData);
        Serial.println(jsonData);
        yield(); // Yield after serial write

        sendConfirmation(from, stationNo, nodeId);
    }
    else if (strcmp(messageType, "identify") == 0) {
        int stationNo = data["station"];
        const char* nodeId = data["nodeId"];
        updateStationActivity(from, stationNo, nodeId);
    }
}

void setup() {
    Serial.begin(115200);
    yield(); // Yield after serial initialization
    
    mesh.setDebugMsgTypes(ERROR | STARTUP);
    mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onDroppedConnection(&droppedConnectionCallback);
    mesh.setRoot(true);
    mesh.setContainsRoot(true);

    Serial.println(F("{\"type\":\"status\",\"message\":\"Mesh network initialized as root\"}"));
    yield(); // Yield after initialization
}

void loop() {
    mesh.update();
    
    unsigned long currentTime = millis();
    if (currentTime - lastConnectionCheck >= CONNECTION_CHECK_INTERVAL) {
        checkStationsConnection();
        lastConnectionCheck = currentTime;
    }
    
    // Always yield at the end of loop
    yield();
}