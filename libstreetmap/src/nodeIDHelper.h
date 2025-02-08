#pragma once
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include <unordered_map>
#include <iostream>
using namespace std;

//sets map to be cleared on next use
void clearIdHelper();

// gets node from ID, if not in map load until value found
const OSMNode* getNodeByID(OSMID nodeID);

// loads all OSMNodes
void loadNodeValueByID();

// gets way from ID, if not in map load until value found
const OSMWay* getWayByID(OSMID nodeID);

// loads all OSMWays
void loadWayValueByID();