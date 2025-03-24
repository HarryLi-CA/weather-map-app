#pragma once

#include "m1.h"
#include "m3.h"
#include "StreetsDatabaseAPI.h"
#include <vector>
#include <stdio.h>
#include <queue>
class Point{
    public:
        IntersectionIdx id = -1;
        StreetSegmentIdx segId = -1;
        Point* parent;
        double gCost = 0, hCost =0, fCost =0;
        bool isPath = false;
        bool isTraversed = false;
        bool correctDir = false;
        LatLon coords, destCoords;

        // for setting destination point
        Point(int id);
        
        //default constructor
        Point(int id, LatLon destCoords, Point* parent);
        ~Point();

        void copy(Point copyPoint);

        void updateFCost();
        vector<Point*> retracePath();
        bool operator <(const Point &x) const {
            return (fCost < x.fCost);
        }
        
        bool operator >(const Point &x) const {
            return (fCost > x.fCost);
        }
};

class PointerCompare {
public:
    template<typename T>
    bool operator()(T *a, T *b) {
    return (*a) > (*b);
    }
};
vector<Point*> calculatePathArray(const double turn_penalty, const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids);
vector<Point*> getNeighbours(Point* current, double turn_penalty, bool isOpposite);