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

#define MAX_STREET_SPEED 36.2 // 36.2m/s = 130km/h

using namespace std;

vector<Point*> calculatePathArray(const double turn_penalty, const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids){
    //cout << "Started Finding Path" << endl;
    clock_t startTime = clock();

    Point* destination = new Point(intersect_ids.second);
    Point* start = new Point(intersect_ids.first, destination->coords, nullptr);
    Point* current = start;

    Heap open;
    vector<Point*> closed={destination};
    vector<pair<Point*, Point*>> segToPoint(osmhelp::getNumStreetSegments(), make_pair(nullptr, nullptr)); // first is correct direction, second is other direction
    vector<double> maxInterGCost(osmhelp::getNumIntersections(), DBL_MAX);

    start->gCost = 0;
    start->destCoords = destination->coords;
    open.add(start);

    //cout<<"Ended basic initial with seconds: "<< (double)(clock() - startTime) / CLOCKS_PER_SEC << endl;

    while(open.length() > 0){
        closed.push_back(open.removeFirst());
        current = closed.back();
        current->isTraversed = true;
        if(current->gCost < maxInterGCost[current->id]){
            double newMaxInterGCost = current->gCost + turn_penalty;
            if(newMaxInterGCost < maxInterGCost[current->id]){
                maxInterGCost[current->id] = newMaxInterGCost;
            }

            if(current->id == destination->id){
                //cout<<"Ended finding path with seconds: "<< (double)(clock() - startTime) / CLOCKS_PER_SEC << endl;
                vector<Point*> path = current->retracePath();
                //cout<<"start basic final with seconds: "<< (double)(clock() - startTime) / CLOCKS_PER_SEC << endl;
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

            vector<Point*> neighbours= getNeighbours(current, turn_penalty, false);
            
            for(Point* neighbour: neighbours){
                bool isGCostHigh = neighbour->gCost > maxInterGCost[neighbour->id];
                if(!isGCostHigh){
                    Point** neighbourInOpen = &(neighbour->correctDir? segToPoint[neighbour->segId].first:segToPoint[neighbour->segId].second);
                    bool hasBeenTraversed = *neighbourInOpen == nullptr || (*neighbourInOpen)->gCost > neighbour->gCost; // closed does not contain current segment
                    if(hasBeenTraversed){
                        if(*neighbourInOpen == nullptr|| neighbour->gCost < (*neighbourInOpen)->gCost){
                            neighbour->updateFCost();
                            if(*neighbourInOpen == nullptr){
                                open.add(neighbour);
                                *neighbourInOpen = neighbour;
                            }
                            else{
                                //cout<<"better duplicate " << endl;
                                (*neighbourInOpen)->copy(*neighbour);
                                open.updateItem(*neighbourInOpen);
                                delete neighbour;
                            }
                        }

                    }
                }
            }
        }
    }
    cout << "Error: Path Not Found" << endl;
    return {};
}

vector<Point*> getNeighbours(Point* current, double turn_penalty, bool isOpposite){
    vector<StreetSegmentIdx> streetSegs = findStreetSegmentsOfIntersection(current->id);
    
    vector<Point*> neighbours;
    StreetSegmentInfo curSegInfo;
    if(current->segId >=0)
        curSegInfo = osmhelp::getStreetSegmentInfo(current->segId);
    for (int i = 0; i < streetSegs.size(); i++)
    {
        StreetSegmentInfo neigbourSegInfo = osmhelp::getStreetSegmentInfo(streetSegs[i]);
        bool isCorrectWay = neigbourSegInfo.from == current->id ^ isOpposite;
        if(!neigbourSegInfo.oneWay || isCorrectWay){ // check one way direction
            IntersectionIdx curIntIdx = findOppositeIntersection(current->id, streetSegs[i]);
            Point* neighbour = new Point(curIntIdx, current->destCoords, current);
            neighbour->segId = streetSegs[i];
            neighbour->correctDir = isCorrectWay;

            double pathTravelTime = 0;
            if(current->parent != nullptr){
                //if streets of segments are not the same, there is a turn
                if(neigbourSegInfo.streetID != curSegInfo.streetID)
                {
                    pathTravelTime += turn_penalty; //apply turn penalty
                }
            }

            neighbour->gCost = current->gCost + findStreetSegmentTravelTime(neighbour->segId) + pathTravelTime;
            neighbours.push_back(neighbour);
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

void Point:: copy(Point copyPoint){
    id = copyPoint.id;
    segId = copyPoint.segId;
    parent = copyPoint.parent;
    gCost = copyPoint.gCost; hCost = copyPoint.hCost; fCost = copyPoint.fCost;
    isPath = copyPoint.isPath;
    isTraversed = copyPoint.isTraversed;
    coords = copyPoint.coords; destCoords = copyPoint.destCoords;
}

void Point::updateFCost(){
    fCost = gCost + hCost;
}

vector<Point*> Point::retracePath(){
    vector<Point> curPath;
    isPath = true;
    if(this->parent!=nullptr){
        // StreetSegmentInfo nextSegmentInfo = osmhelp::getStreetSegmentInfo(this->segId);
        // cout << nextSegmentInfo.speedLimit << endl;
        vector<Point*> curPath = this->parent->retracePath();
        curPath.push_back(this);
        return curPath;
    }
    else{
        return {this};
    }
}