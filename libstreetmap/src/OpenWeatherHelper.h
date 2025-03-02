#pragma once
#include <limits.h>
#include <vector>
#include <cmath>
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"

#define PI 3.14159265358979323846264338327950288
#define TILEWIDTH 256
#define MAXZ 9
using namespace std;

struct MapTileData {
    int x,
    int y,
    int z,
}

// Get global coordiantes of mercator projection from latitude and longitude
pair<double,double> getGlobalCoordsFromLatLon(LatLon l, double mapWidth);

vector<MapTileData> getMapTilesFromBounds (LatLonBounds currentScope, int pixelHeight, int pixelLength);