#include "m1.h'
#include "m3.h"
#include "PathFinding.h"
#include <vector>
#include <stdio.h>

using namespace std;

vector<Point> calculatePathArray(const double turn_penalty, const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids){
    Point destination = Point(intesectionIdx.second);
    Point start = Point(startNode, destionation.coords, nullptr);
    Point current;

    priority_queue <Point> open = priority_queue <Point>();
    vector<Point> closed={};

    start.gCost = 0;
    start.destCoords = destination.coordinates;
    open.push(start);
    while(open.size() > 0){
        current = open.removeFirst();
        closed.push(current);
        if(current.id == destinationNode){
            List<Point> path = current.retracePath();
            return path;
        }

        vector<Point> neighbours= getNeighbours(current);
        
        for(Point neighbour: neighbours){
            //if(closed.where((element) => element.name==neighbour.name).isEmpty){
                double newMovementCostToNeighbour = current.gCost +
                findDistanceBetweenTwoPoints(
                current.coords,
                neighbour.coords,
                );

                /*
                future optimization
                List<Point> neighbourInOpenList = open.items.where((element) => element.name==neighbour.name).toList();

                if(neighbourInOpenList.isEmpty || newMovementCostToNeighbour<neighbourInOpenList[0].gCost!){
                neighbour.gCost = newMovementCostToNeighbour;
                neighbour.hCost = calcDistance(
                    neighbour.coordinates[0],
                    neighbour.coordinates[1],
                    destination.coordinates[0],
                    destination.coordinates[1],
                );
                */
                neighbour.updateFCost();
                neighbour.parent = current;

                
                // if(neighbourInOpenList == null||neighbourInOpenList.length<=0){
                    open.push(neighbour);
                // }
                // else{
                //     int indexOfNeighbour =open.items.indexOf(neighbourInOpenList[0]);
                //     open.items[indexOfNeighbour] = neighbour;
                //     open.updateItem(neighbour);
                // }
            //}
        }
    }
    cout << 'Error: Path Not Found' << endl;
    return {};
}

vector<Point> getNeighbours(Point current){

    return {};
}

// for setting destination point
Point::Point(int id){
    this.id = id;
    this.coords = getIntersectionPosition(id);
}

//default constructor
Point::Point(int id, LatLon destCoords, Point* parent){
    this.id = id;
    this.coords = getIntersectionPosition(id);
    this.destCoords=  destCoords;
    this.parent = parent;
    hCost = findDistanceBetweenTwoPoints(coords, destCoords);
    fCost = gCost + hCost;
}
Point::~Point(){
    if(parent!=nullptr){
        delete parent;
        parent == nullptr;
    }
}

Point::void updateFCost(){
    fCost = gCost + hCost;
}
Point::vector<Point> retracePath(){
    if(this.parent!=nullptr){
        vector<Point> curPath = retracePath(this.parent);
        curPath.push_back(this);
        return curPath;
    }
    else{
        return {this};
    }
}