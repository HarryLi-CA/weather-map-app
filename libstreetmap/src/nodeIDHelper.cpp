#include "nodeIDHelper.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include <unordered_map>
#include <iostream>
using namespace std;

int curNodeIdx = 0;
int totalNodes = -1;
unordered_map<uint64_t, const OSMNode*> nodeIDReference;

const OSMNode* getNodeByID(OSMID nodeID){
    if(totalNodes==-1)
        totalNodes = getNumberOfNodes();

    if(nodeIDReference.find(uint64_t(nodeID))!= nodeIDReference.end()) // return if alreadly loaded
        return nodeIDReference[uint64_t(nodeID)];
    OSMID curID;
    for(;curNodeIdx <totalNodes || curID == nodeID; curNodeIdx++){ // load until node is found
        const OSMNode* node= getNodeByIndex(curNodeIdx);
        curID= node->id();
        nodeIDReference[uint64_t(curID)] = node;
    }
    if(curNodeIdx == totalNodes)
        cout<< "reached end of nodes" << endl;
    return nodeIDReference[uint64_t(nodeID)];
}
void loadNodeValueByID(){
    if(totalNodes==-1)
        totalNodes = getNumberOfNodes();

    for(; curNodeIdx < totalNodes; curNodeIdx++){
        const OSMNode* node= getNodeByIndex(curNodeIdx);
        OSMID nodeID= node->id();
        nodeIDReference[uint64_t(nodeID)] = node;
    }
}

int curWayIdx = 0;
int totalWays = -1;
unordered_map<uint64_t, const OSMWay*> wayIDReference;

const OSMWay* getWayByID(OSMID wayID){
    if(totalWays==-1)
        totalWays = getNumberOfWays();

    if(wayIDReference.find(uint64_t(wayID))!= wayIDReference.end()) // return if alreadly loaded
        return wayIDReference[uint64_t(wayID)];

    OSMID curID;
    for(;curWayIdx <totalWays || curID == wayID; curWayIdx++){ // load until way is found
        const OSMWay* way = getWayByIndex(curWayIdx);
        curID= way->id();
        wayIDReference[uint64_t(curID)] = way;
    }
    if(curWayIdx == totalWays)
        cout<< "reached end of ways" << endl;
    return wayIDReference[uint64_t(wayID)];
}
void loadWayValueByID(){
    if(totalWays == -1)
        totalWays = getNumberOfWays();

    for(; curWayIdx < totalWays; curWayIdx++){
        const OSMWay* way= getWayByIndex(curWayIdx);
        OSMID wayID= way->id();
        wayIDReference[uint64_t(wayID)] = way;
    }
}



