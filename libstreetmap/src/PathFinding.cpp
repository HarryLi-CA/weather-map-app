#include "PathFinding.h"
#include "m1.h"
#include "m3.h"
#include "OSMAPIHelper.h"
#include <vector>
#include <stdio.h>
#include <queue>

#define MAX_STREET_SPEED 200

using namespace std;

vector<Point> calculatePathArray(const double turn_penalty, const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids){
    Point destination = Point(intersect_ids.second);
    Point start = Point(intersect_ids.first, destination.coords, nullptr);
    Point current = start;

    priority_queue <Point> open;
    vector<Point> closed={};

    start.gCost = 0;
    start.destCoords = destination.coords;
    open.push(start);
    while(open.size() > 0){
        current = open.top(); open.pop();
        closed.push_back(current);
        if(current.id == destination.id){
            vector<Point> path = current.retracePath();
            return path;
        }

        vector<Point> neighbours= getNeighbours(current, turn_penalty);
        
        for(Point neighbour: neighbours){
            //if(closed.where((element) => element.name==neighbour.name).isEmpty){
                //double newMovementCostToNeighbour = neighbour.gCost;
                /*
                future optimization
                vector<Point> neighbourInOpenvector = open.items.where((element) => element.name==neighbour.name).tovector();

                if(neighbourInOpenvector.isEmpty || newMovementCostToNeighbour<neighbourInOpenvector[0].gCost!){
                neighbour.gCost = newMovementCostToNeighbour;
                neighbour.hCost = calcDistance(
                    neighbour.coords[0],
                    neighbour.coords[1],
                    destination.coords[0],
                    destination.coords[1],
                );
                */
                neighbour.updateFCost();

                
                // if(neighbourInOpenvector == null||neighbourInOpenvector.length<=0){
                    open.push(neighbour);
                // }
                // else{
                //     int indexOfNeighbour =open.items.indexOf(neighbourInOpenvector[0]);
                //     open.items[indexOfNeighbour] = neighbour;
                //     open.updateItem(neighbour);
                // }
            //}
        }
    }
    cout << "Error: Path Not Found" << endl;
    return {};
}

vector<Point> getNeighbours(Point current, double turn_penalty){
    vector<StreetSegmentIdx> streetSegs = findStreetSegmentsOfIntersection(current.id);
    
    vector<Point> neighbours;
    for (int i = 0; i < streetSegs.size(); i++)
    {
        StreetSegmentInfo currentSegmentInfo = osmhelp::getStreetSegmentInfo(streetSegs[i]);
        if(currentSegmentInfo.from != current.id){ // check one way direction
            IntersectionIdx curIntIdx = findOppositeIntersection(current.id, streetSegs[i]);
            Point curPoint = Point(curIntIdx, current.destCoords, &current);
            curPoint.segId = streetSegs[i];

            double pathTravelTime = 0;
            if(current.parent != nullptr){
                StreetSegmentInfo nextSegmentInfo = osmhelp::getStreetSegmentInfo(current.segId);
                //if streets of segments are not the same, there is a turn
                if(currentSegmentInfo.streetID != nextSegmentInfo.streetID)
                {
                    pathTravelTime += turn_penalty; //apply turn penalty
                }
            }

            curPoint.gCost = current.gCost + findStreetSegmentTravelTime(curPoint.segId);
        }
    }
    
    return neighbours;
}

// for setting destination point
Point::Point(int id){
    this->id = id;
    this->coords = osmhelp::getIntersectionPosition(id);
}

//default constructor
Point::Point(int id, LatLon destCoords, Point* parent){
    this->id = id;
    this->coords = osmhelp::getIntersectionPosition(id);
    this->destCoords=  destCoords;
    this->parent = parent;
    hCost = findDistanceBetweenTwoPoints(coords, destCoords)/MAX_STREET_SPEED;
    fCost = gCost + hCost;
}

Point::~Point(){
    if(parent!=nullptr){
        delete parent;
        parent == nullptr;
    }
}

void Point::updateFCost(){
    fCost = gCost + hCost;
}

vector<Point> Point::retracePath(){
    if(this->parent!=nullptr){
        vector<Point> curPath = this->parent->retracePath();
        curPath.push_back(*this);
        return curPath;
    }
    else{
        return {*this};
    }
}