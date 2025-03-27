#pragma once

#include "StreetsDatabaseAPI.h"
#include <vector>
#include <stdio.h>
#include <queue>
#include <mutex>
#include <utility>
#include <limits>
#include "Heap.h"
#include "SegmentTree.h"
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

    vector<Point*> intersectionsVisited1;
    vector<Point*> intersectionsVisited2;

    mutex pathFoundMutex;
    mutex newPointMutex;

    volatile bool optimalPathFound = false;
    volatile bool possiblePathFound = false;
    volatile double maxCost = DBL_MAX;

    bool fullyOptimal = true; // 0.5% time save to flip off
    bool bidirectionalEasy = false; //flipping this might only save 3.57% time
    
    Point* pathCorrect = nullptr;
    Point* pathOpposite = nullptr;

    MinSegmentTree* mSTCorrect = nullptr;
    MinSegmentTree* mSTOpposite = nullptr;

    Point* minGCostPCorrect = nullptr;
    Point* minGCostPOpposite = nullptr;

    PathFinder();
    ~PathFinder();
    vector<Point*> calculatePathArray(const double turn_penalty, const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids);
    
    void findHalfPath(const double turn_penalty, IntersectionIdx begin, IntersectionIdx end, 
    Heap& open, vector<Point*>& closed, pair<mutex,vector<pair<Point*, Point*>>>& segToPoint,
    vector<double>& maxInterGCost, pair<mutex,vector<pair<Point*, Point*>>>& segToPoint2,
    Point*& path, Point*& path2, vector<Point*>& intersectionsVisited, vector<Point*>& opIntersectionsVisited, const bool isOpposite);
    
    vector<Point*> getNeighbours(Point* current, double turn_penalty, bool isOpposite);

    double getMinGCost(bool isOpposite);
};