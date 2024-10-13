#include <painlessMesh.h>
#include <ArduinoJson.h>

#define MESH_PREFIX     "HIKING"
#define MESH_PASSWORD   "meshPassword"
#define MESH_PORT       5555

painlessMesh mesh;

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

        Serial.printf("Received data from Station %d, NodeID: %s\n", stationNo, nodeId.c_str());
        // Print the received data to serial port in JSON format
        String jsonData;
        serializeJson(data, jsonData);  // Serialize the data back to JSON format
        Serial.println(jsonData);       // Send the JSON data over serial
        
        

        // Send confirmation back to the sender
        sendConfirmation(from, stationNo, nodeId);
    }
}

// Function to request data from a specific node
void requestDataFromNode(uint32_t nodeId) {
    DynamicJsonDocument doc(128);
    doc["type"] = "request";
    doc["requestType"] = "data";

    String msg;
    serializeJson(doc, msg);
    mesh.sendSingle(nodeId, msg);
}

void setup() {
    Serial.begin(115200);
    
    mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);
    mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
    mesh.onReceive(&receivedCallback);
    mesh.setRoot(true);
    mesh.setContainsRoot(true);

    Serial.println("Mesh network initialized. This node is set as root.");
}

void loop() {
    mesh.update();
}