#pragma once

#include "m1.h"
#include "m3.h"
#include "StreetsDatabaseAPI.h"
#include <vector>
#include <stdio.h>
#include <queue>
class Point{
    public:
        int id;
        Point* parent;
        double gCost = 0, hCost =0, fCost =0;
        bool isTurn = false;
        LatLon coords, destCoords;

        // for setting destination point
        Point(int id);
        
        //default constructor
        Point(int id, LatLon destCoords, Point* parent);
        ~Point();

        void updateFCost();
        vector<Point> retracePath();
        bool operator <(const Point &x) const {
            return (fCost < x.fCost);
        }
        
        bool operator >(const Point &x) const {
            return (fCost > x.fCost);
        }
};
vector<Point> calculatePathArray(const double turn_penalty, const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids);
vector<Point> getNeighbours(Point current);