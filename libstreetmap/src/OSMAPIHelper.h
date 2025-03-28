#pragma once
#include <thread>
#include <mutex>
#include <utility>
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"

#include "ezgl/application.hpp"

namespace osmhelp{
    void load();
    void reset();
    int getNumIntersections(); //returns total number of intersections on a map
    int getNumStreetSegments(); //returns total number of street segments on a map
    int getNumStreets(); //returns total number of streets on a map
    ezgl::point2d getPoint2dOfIntersections(const IntersectionIdx idx);
    StreetSegmentInfo getStreetSegmentInfo(const StreetSegmentIdx idx);
    StreetSegmentInfo* getStreetSegmentInfoPtr(const StreetSegmentIdx idx);
    LatLon getIntersectionPosition(const IntersectionIdx idx);
    double getSpeedLimitMaximum(); //calculates maximum speed limit for a map
    double getSpeedLimitMinimum();
}