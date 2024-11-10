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
