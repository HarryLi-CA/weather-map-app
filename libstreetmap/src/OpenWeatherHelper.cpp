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

pair<double,double> getGlobalCoordsFromLatLon(LatLon l, double mapWidth){
    double latitude    = 43.6532; // (φ)
    double longitude   = -79.3832;   // (λ)

    double mapWidth    = 8;
    double mapHeight   = 8;

    double x = (longitude+180)*(mapWidth/360); // x val

    double mercN = log(tan((PI/4)+(latitude*kDegreeToRadian/2)));
    double y     = (mapHeight/2)-(mapWidth*mercN/(2*PI)); // get y value without offset
    return make_pair(x, y);
}

vector<MapTileData> getMapTilesFromBounds (LatLonBounds currentScope, int pixelHeight, int pixelLength){
    int optimalTilesWide = pixelHeight / TILEWIDTH + 1; // minimum tiles for max resolution 
    int optimalTilesHigh = pixelLength / TILEWIDTH + 1;
    
    int z = 0;
    int zoom = 1;

    pair<double,double> minGlobalCoords = getGlobalCoordsFromLatLon(currentScope.min, zoom);
    pair<double,double> maxGlobalCoords = getGlobalCoordsFromLatLon(currentScope.max, zoom);

    double currentTilesWide = maxGlobalCoords.first - minGlobalCoords.first;
    double currentTilesHigh = maxGlobalCoords.second - minGlobalCoords.second;
    for(;z < MAXZ || (currentTilesWide > optimalTilesWide && optimalTilesHigh>currentTilesHigh); z++){
        currentTilesWide*=2;
        currentTilesHigh*=2;
        zoom*=2
    }

    //now get real coords
    minGlobalCoords = getGlobalCoordsFromLatLon(currentScope.min, zoom);
    maxGlobalCoords = getGlobalCoordsFromLatLon(currentScope.max, zoom);

    vector<MapTileData> out;
    for(int i = minGlobalCoords.first; i < maxGlobalCoords.first; i++){ 
        for(int j = minGlobalCoords.first; j < maxGlobalCoords.second; j++){
            MapTileData m = {.x = i, .y = j, .z = z};
            out.push_back(m);
        }
    }
    return out;
}