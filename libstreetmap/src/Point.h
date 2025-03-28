#pragma once

#include "StreetsDatabaseAPI.h"
#include <vector>
#include <stdio.h>
#include <queue>
#include <atomic>
#include <vector>
#include "ezgl/application.hpp"

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
        int segTreeIndex = -1;
        ezgl::point2d coords, destCoords;

        // for setting destination point
        Point(int id);
        
        //default constructor
        Point(int id, ezgl::point2d destCoords_, Point* parent_);
        ~Point();

        void copy(Point copyPoint);

        static double calculateHCost(ezgl::point2d point1, ezgl::point2d point2);
        void updateHCost();
        
        void updateFCost();
        std::vector<Point*> retracePath(std::vector<Point*> curPath, bool isOpposite);
        bool operator <(const Point &x) const {
            return (fCost < x.fCost);
        }
        
        bool operator >(const Point &x) const {
            return (fCost > x.fCost);
        }
};