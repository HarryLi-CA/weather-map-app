#include "PathFinding.h"
#include "m1.h"
#include "m3.h"
#include "Heap.h"
#include "OSMAPIHelper.h"
#include <vector>
#include <stdio.h>
#include <queue>
#include <ctime>

#define MAX_STREET_SPEED 36.2 // m/s = 130km/h

using namespace std;

vector<Point*> calculatePathArray(const double turn_penalty, const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids){
    // cout << "Started Finding Path" << endl;
    // clock_t startTime = clock();

    Point* destination = new Point(intersect_ids.second);
    Point* start = new Point(intersect_ids.first, destination->coords, nullptr);
    Point* current = start;

    Heap open;
    vector<Point*> closed={destination};
    vector<Point*> segToPoint(osmhelp::getNumStreetSegments(), nullptr);

    start->gCost = 0;
    start->destCoords = destination->coords;
    open.add(start);
    while(open.length() > 0){
        closed.push_back(open.removeFirst());
        current = closed.back();
        current->isTraversed = true;

        if(current->id == destination->id){
            //cout<<"Ended finding with seconds: "<< (double)(clock() - startTime) / CLOCKS_PER_SEC << endl;
            vector<Point*> path = current->retracePath();
            for(int i = 0; i <closed.size(); i++)
                if(!closed[i]->isPath)
                    delete closed[i];
            for(int i = 0; i <open.items.size(); i++)
                delete open.items[i];
            open.items.clear();
            open.currentItemCount = 0;
            
            //cout<<"Ended finding with seconds: "<< (double)(clock() - startTime) / CLOCKS_PER_SEC << endl;
            return path;
        }

        vector<Point*> neighbours= getNeighbours(current, turn_penalty);
        
        for(Point* neighbour: neighbours){
            if(segToPoint[neighbour->segId] == nullptr || !segToPoint[neighbour->segId]->isTraversed){ // closed does not contain current segment

                Point* neighbourInOpen = segToPoint[neighbour->segId];

                if(neighbourInOpen == nullptr|| neighbour->gCost < neighbourInOpen->gCost){
                    neighbour->updateFCost();
                    if(neighbourInOpen == nullptr){
                        open.add(neighbour);
                        segToPoint[neighbour->segId] = neighbour;
                    }
                    else{
                        segToPoint[neighbour->segId]->gCost = neighbour->gCost;
                        segToPoint[neighbour->segId]->parent = neighbour->parent;
                        segToPoint[neighbour->segId]->coords = neighbour->coords;
                        segToPoint[neighbour->segId]->id = neighbour->id;
                        segToPoint[neighbour->segId]->updateFCost();
                        open.updateItem(segToPoint[neighbour->segId]);
                        delete neighbour;
                    }
                }

            }
        }
    }
    cout << "Error: Path Not Found" << endl;
    return {};
}

vector<Point*> getNeighbours(Point* current, double turn_penalty){
    vector<StreetSegmentIdx> streetSegs = findStreetSegmentsOfIntersection(current->id);
    
    vector<Point*> neighbours;
    for (int i = 0; i < streetSegs.size(); i++)
    {
        StreetSegmentInfo currentSegmentInfo = osmhelp::getStreetSegmentInfo(streetSegs[i]);
        if(!(currentSegmentInfo.oneWay && currentSegmentInfo.from != current->id)){ // check one way direction
            IntersectionIdx curIntIdx = findOppositeIntersection(current->id, streetSegs[i]);
            Point* curPoint = new Point(curIntIdx, current->destCoords, current);
            curPoint->segId = streetSegs[i];

            double pathTravelTime = 0;
            if(current->parent != nullptr){
                StreetSegmentInfo nextSegmentInfo = osmhelp::getStreetSegmentInfo(current->segId);
                //if streets of segments are not the same, there is a turn
                if(currentSegmentInfo.streetID != nextSegmentInfo.streetID)
                {
                    pathTravelTime += turn_penalty; //apply turn penalty
                }
            }

            curPoint->gCost = current->gCost + findStreetSegmentTravelTime(curPoint->segId) + pathTravelTime;
            neighbours.push_back(curPoint);
        }
    }
    
    return neighbours;
}

// for setting destination point
Point::Point(int id){
    this->id = id;
    this->coords = osmhelp::getIntersectionPosition(id);
    this->parent = nullptr;
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
}

void Point::updateFCost(){
    fCost = gCost + hCost;
}

vector<Point*> Point::retracePath(){
    vector<Point> curPath;
    isPath = true;
    if(this->parent!=nullptr){
        vector<Point*> curPath = this->parent->retracePath();
        curPath.push_back(this);
        return curPath;
    }
    else{
        return {this};
    }
}