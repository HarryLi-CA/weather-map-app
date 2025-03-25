#pragma once

#include "m1.h"
#include "m3.h"
#include "StreetsDatabaseAPI.h"
#include <vector>
#include <stdio.h>
#include <queue>
#include <mutex>
#include <utility>
#include <limits>
#include "Heap.h"
#include "Point.h"


class PathFinder{
    public:
    Heap open1;
    Heap open2;

    vector<Point*> closed1;
    vector<Point*> closed2;

    pair<mutex, vector<pair<Point*, Point*>>> segToPointCorrect;
    pair<mutex, vector<pair<Point*, Point*>>> segToPointOpposite;

    vector<double> maxInterGCostCorrect;
    vector<double> maxInterGCost2;

    mutex pathFoundMutex;
    mutex newPointMutex;

    volatile bool pathFound = false;
    volatile double maxCost = DBL_MAX;

    bool fullyOptimal = false;
    bool bidirectionalEasy = true;
    
    Point* pathCorrect = nullptr;
    Point* pathOpposite = nullptr;

    Point* minGCostP1 = nullptr;
    Point* minGCostP2 = nullptr;

    double minGCost1 = -1;
    double minGCost2 = -1;

    PathFinder();
    ~PathFinder();
    vector<Point*> calculatePathArray(const double turn_penalty, const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids);
    
    void findHalfPath(const double turn_penalty, IntersectionIdx begin, IntersectionIdx end, 
        Heap& open, vector<Point*>& closed, pair<mutex,vector<pair<Point*, Point*>>>& segToPoint,
        vector<double>& maxInterGCost, pair<mutex,vector<pair<Point*, Point*>>>& segToPoint2,
        Point*& path, Point*& path2, bool isOpposite);
    
    vector<Point*> getNeighbours(Point* current, double turn_penalty, bool isOpposite);

    double getMinGCost(bool isOpposite);
};