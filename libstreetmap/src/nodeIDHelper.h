#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include <unordered_map>
#include <iostream>
using namespace std;
const OSMNode* getNodeByID(OSMID nodeID);
void loadNodeValueByID();
const OSMWay* getWayByID(OSMID nodeID);
void loadWayValueByID();