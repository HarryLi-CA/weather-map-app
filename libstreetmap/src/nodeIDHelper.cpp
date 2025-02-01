#include "nodeIDHelper.h"
#include <unordered_map>
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
using namespace std;

int curIdx = 0;
int totalNodes = 0;
bool isCalled = false;
unordered_map<uint64_t, const OSMNode*> nodeIDReference;
const OSMNode* getNodeByID(OSMID nodeID){
    if(!isCalled)
        totalNodes = getNumberOfNodes();

    if(nodeIDReference.find(uint64_t(nodeID))!= nodeIDReference.end()) // return if alreadly loaded
        return nodeIDReference[uint64_t(nodeID)];

    OSMID curID;
    for(;curIdx <totalNodes || curID == nodeID; curIdx++){ // load until node is found
        const OSMNode* node= getNodeByIndex(curIdx);
        curID= node->id();
        nodeIDReference[uint64_t(nodeID)] = node;
    }
    return nodeIDReference[uint64_t(nodeID)];
}
void loadNodeValueByID(){
    if(!isCalled)
        totalNodes = getNumberOfNodes();

    for(; curIdx < totalNodes; curIdx++){
        const OSMNode* node= getNodeByIndex(curIdx);
        OSMID nodeID= node->id();
        nodeIDReference[uint64_t(nodeID)] = node;
    }
}