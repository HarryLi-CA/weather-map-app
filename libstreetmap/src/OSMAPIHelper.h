#pragma once
#include <thread>
#include <mutex>
#include <utility>
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"


namespace osmhelp{
    void reset();
    int getNumIntersections(); //returns total number of intersections on a map
    int getNumStreetSegments(); //returns total number of street segments on a map
    int getNumStreets(); //returns total number of streets on a map
    StreetSegmentInfo getStreetSegmentInfo(StreetSegmentIdx idx);
    LatLon getIntersectionPosition(IntersectionIdx idx);
    double getSpeedLimitMaximum(); //calculates maximum speed limit for a map
}