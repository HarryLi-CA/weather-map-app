#pragma once

#include "m1.h"
#include "m3.h"
#include "StreetsDatabaseAPI.h"
#include <vector>
#include <stdio.h>
#include <queue>
#include <atomic>
#include "Heap.h"
#include "Point.h"

#define MAX_STREET_SPEED 36.2// 36.2m/s = 130km/h

class Point{
    public:
        IntersectionIdx id = -1;
        StreetSegmentIdx segId = -1;
        Point* parent;
        double gCost = 0, hCost =0, fCost =0;
        bool isPath = false;
        bool isTraversed = false;
        bool correctDir = false;

        int index = -1;
        LatLon coords, destCoords;

        // for setting destination point
        Point(int id);
        
        //default constructor
        Point(int id, LatLon destCoords_, Point* parent_);
        ~Point();

        void copy(Point copyPoint);

        void updateFCost();
        vector<Point*> retracePath(vector<Point*> curPath, bool isOpposite);
        bool operator <(const Point &x) const {
            return (fCost < x.fCost);
        }
        
        bool operator >(const Point &x) const {
            return (fCost > x.fCost);
        }
};