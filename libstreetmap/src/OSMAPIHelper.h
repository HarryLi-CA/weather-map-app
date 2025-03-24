#pragma once
#include <thread>
#include <mutex>
#include <utility>
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"


namespace osmhelp{
    void load();
    void reset();
    int getNumIntersections();
    int getNumStreetSegments();
    int getNumStreets();
    StreetSegmentInfo getStreetSegmentInfo(StreetSegmentIdx idx);
    LatLon getIntersectionPosition(IntersectionIdx idx);
}