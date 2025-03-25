#include "PathFinding.h"
#include "m1.h"
#include "m3.h"
#include "Heap.h"
#include "OSMAPIHelper.h"
#include <vector>
#include <stdio.h>
#include <queue>
#include <ctime>
#include <limits>
#include <atomic>
#include "ezgl/application.hpp"

// for setting destination point
Point::Point(int id_){
    this->id = id_;
    this->coords = osmhelp::getPoint2dOfIntersections(id_);
    this->parent = nullptr;
}

//default constructor
Point::Point(int id_, ezgl::point2d destCoords_, Point* parent_){
    this->id = id_;
    this->coords = osmhelp::getPoint2dOfIntersections(id_);
    this->destCoords=  destCoords_;
    this->parent = parent_;
    //hCost = findDistanceBetweenTwoPoints(coords, destCoords_)/MAX_STREET_SPEED;
    fCost = gCost + hCost;
}

Point::~Point(){
}

void Point:: copy(Point copyPoint){
    id = copyPoint.id;
    segId = copyPoint.segId;
    parent = copyPoint.parent;
    gCost = copyPoint.gCost; hCost = copyPoint.hCost; fCost = copyPoint.fCost;
    isPath = copyPoint.isPath;
    isTraversed = copyPoint.isTraversed;
    coords = copyPoint.coords; destCoords = copyPoint.destCoords;
}


void Point::updateHCost(){
    hCost = magnitude(coords- destCoords)/MAX_STREET_SPEED;
}

void Point::updateFCost(){
    fCost = gCost + hCost;
}

vector<Point*> Point::retracePath(vector<Point*> curPath, bool isOpposite){
    isPath = true;
    if(this->parent!=nullptr){
        // StreetSegmentInfo nextSegmentInfo = osmhelp::getStreetSegmentInfo(this->segId);
        // cout << nextSegmentInfo.speedLimit << endl;
        if(!isOpposite){
            curPath = this->parent->retracePath(curPath, isOpposite);
            curPath.push_back(this);
        }
        else{
            curPath.push_back(this);
            curPath = this->parent->retracePath(curPath, isOpposite);
        }
        return curPath;
    }
    curPath.push_back(this);
    return curPath;
}