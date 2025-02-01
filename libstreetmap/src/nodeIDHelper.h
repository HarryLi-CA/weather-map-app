#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include <unordered_map>
using namespace std;
const OSMNode* getNodeByID(OSMID nodeID);
void loadNodeValueByID();