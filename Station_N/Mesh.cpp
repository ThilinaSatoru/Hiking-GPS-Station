#include "Config.h"

Task taskSendData(TASK_SECOND * 5, TASK_FOREVER, &sendMeshData);

void setupMesh() {
    mesh.setDebugMsgTypes(ERROR | STARTUP);
    mesh.init(MESH_PREFIX, MESH_PASSWORD, &meshScheduler, MESH_PORT);
    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onChangedConnections(&changedConnectionCallback);
    mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

    meshScheduler.addTask(taskSendData);
    taskSendData.enable();
}

String getMeshData() {
    nodeId = mesh.getNodeId();
    StaticJsonDocument<200> data_json;

    data_json["type"] = "data";
    data_json["station"] = STATION_NUMBER;
    data_json["nodeId"] = String(nodeId);
    data_json["lat"] = gps.location.lat();
    data_json["lng"] = gps.location.lng();
    data_json["emergency"] = EMERGENCY;
    data_json["emergencyDuration"] = getEmergencyDurationMinutes();
    data_json["battery"] = batteryPercentage;

    Serial.print("Sending : ");
    serializeJson(data_json, Serial);
    Serial.println();

    String output;
    serializeJson(data_json, output);
    return output;
}

void sendMeshData() {
    mesh.sendBroadcast(getMeshData());
}

void receivedCallback(uint32_t from, String &msg) {
  StaticJsonDocument<200> jsonDoc;
  DeserializationError error = deserializeJson(jsonDoc, msg);

  if (!error) {
    String messageType = jsonDoc["type"];
    if (messageType == "confirmation") {
        int confirmedStation = jsonDoc["station"];
        String confirmedNodeId = jsonDoc["nodeId"];

        if (confirmedStation == STATION_NUMBER && confirmedNodeId == String(nodeId)) {
            Serial.println("Received confirmation from Main.");
            EMERGENCY = false;
            updateLEDState();  // This will update LED1 state
        }
    } else if (messageType == "request") {
        sendMeshData();
    }
  }
}

void newConnectionCallback(uint32_t nodeId) {
    // Handle new connection
}

void changedConnectionCallback() {
    // Handle changed connections
}

void nodeTimeAdjustedCallback(int32_t offset) {
    // Handle time adjustment
}