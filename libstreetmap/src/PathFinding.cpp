
#include "PathFinding.h"
#include "m1.h"
#include "m3.h"
#include <vector>
#include <stdio.h>
#include <queue>

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

        vector<Point> neighbours= getNeighbours(current);
        
        for(Point neighbour: neighbours){
            //if(closed.where((element) => element.name==neighbour.name).isEmpty){
                double newMovementCostToNeighbour = neighbour.gCost;
                if(neighbour.isTurn)
                    newMovementCostToNeighbour += turn_penalty;
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
                neighbour.parent = &current;

                
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

vector<Point> getNeighbours(Point current){

    return {};
}

// for setting destination point
Point::Point(int id){
    this->id = id;
    this->coords = getIntersectionPosition(id);
}

//default constructor
Point::Point(int id, LatLon destCoords, Point* parent){
    this->id = id;
    this->coords = getIntersectionPosition(id);
    this->destCoords=  destCoords;
    this->parent = parent;
    hCost = findDistanceBetweenTwoPoints(coords, destCoords);
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