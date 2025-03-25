#include "PathFinding.h"
#include "m1.h"
#include "m3.h"
#include "Heap.h"
#include "Point.h"
#include "OSMAPIHelper.h"
#include <vector>
#include <stdio.h>
#include <queue>
#include <ctime>
#include <limits>
#include <atomic>
#include <functional>
#include <algorithm>

using namespace std;

static clock_t totalTime = 0;
/*
16391ms but took 49981.4ms. // djekstra

16391ms but took 68131.3ms

non optimal
16391ms but took 25804.1ms. //djeekstra
16391ms but took 38236ms

construct destruct total time is 855ms
*/

PathFinder::PathFinder(){
    //clock_t t = clock();
    segToPointCorrect.second.resize(osmhelp::getNumStreetSegments(), make_pair(nullptr, nullptr));
    segToPointOpposite.second.resize(osmhelp::getNumStreetSegments(), make_pair(nullptr, nullptr));
    maxInterGCostCorrect.resize(osmhelp::getNumIntersections(), DBL_MAX);
    maxInterGCost2.resize(osmhelp::getNumIntersections(), DBL_MAX);
    //cout << "done start: " <<(clock() -t)/(double)CLOCKS_PER_SEC << endl;
}
PathFinder::~PathFinder(){
    //clock_t t = clock();
    //cout<<"begin free" << endl;
    for(int i =0; i < closed1.size(); i++){
        //if(!closed1[i]->isPath)
            delete closed1[i];
        closed1[i] = nullptr;
    }
    //cout<<"begin free 2" << endl;
    for(int i =0; i < closed2.size(); i++){
        //if(!closed2[i]->isPath)
            delete closed2[i];
        closed2[i] = nullptr;
    }
    //cout<< "free closed" << endl;
    for(int i =0; i < open1.items.size(); i++)
        delete open1.items[i];
    for(int i =0; i < open2.items.size(); i++)
        delete open2.items[i];
    //cout << "done delete: " <<(clock() -t)/(double)CLOCKS_PER_SEC << endl;
    
    
}

vector<Point*> PathFinder::calculatePathArray(const double turn_penalty, const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids){
    //cout << "searching for path "<<intersect_ids.first << " " << intersect_ids.second <<'\n';
    //clock_t t = clock();
    thread t1(&PathFinder::findHalfPath, this, turn_penalty, intersect_ids.first, intersect_ids.second, 
        ref(open1), ref(closed1), ref(segToPointCorrect),
        ref(maxInterGCostCorrect), ref(segToPointOpposite),
        ref(path1), ref(path2), false);
    thread t2(&PathFinder::findHalfPath, this, turn_penalty, intersect_ids.second, intersect_ids.first, 
        ref(open2), ref(closed2), ref(segToPointOpposite),
        ref(maxInterGCost2), ref(segToPointCorrect),
        ref(path2), ref(path1), true);
    t1.join();
    t2.join();
    //totalTime +=(clock() -t);
    //cout << "path found: " <<totalTime/(double)CLOCKS_PER_SEC<< endl;
    //cout << "got path" << endl;
    vector<Point*> out = {};
    if(path1 == nullptr && path2 == nullptr)
        cout << "Error: Path Not Found\n";
    if(path1 !=nullptr)
        out = path1->retracePath(out, false);
    //cout << "path1 complete" << endl;
    if(path2 !=nullptr)
        out = path2->retracePath(out, true);
    //cout << "path2 complete" << endl;
    return out;
}

void PathFinder::findHalfPath(const double turn_penalty, IntersectionIdx begin, IntersectionIdx end, 
    Heap& open, vector<Point*>& closed, pair<mutex,vector<pair<Point*, Point*>>>& segToPoint,
    vector<double>& maxInterGCost, pair<mutex,vector<pair<Point*, Point*>>>& segToPoint2,
    Point*& path, Point*& path2, const bool isOpposite){
        
    newPointMutex.lock();
    Point* destination = new Point(end);
    Point* start = new Point(begin, destination->coords, nullptr);
    newPointMutex.unlock();

    Point* current = start;

    closed.push_back(destination);

    start->gCost = 0;
    start->destCoords = destination->coords;
    open.add(start);

    //cout<<"Ended basic initial with seconds: "<< (double)(clock() - startTime) / CLOCKS_PER_SEC << endl;

    while(open.length() > 0){
        closed.push_back(open.removeFirst());
        current = closed.back();



        if(current->fCost>maxCost)
            break;

        // double oppositeMin = getMinGCost(!isOpposite);
        // if(oppositeMin >= 0 && current->gCost + oppositeMin > maxCost)
        //     continue;

        if(pathFound){
            return;
        }
        current->isTraversed = true;

        if(current->gCost < maxInterGCost[current->id]){
            Point* opposing = nullptr;
            if(current->segId >= 0){
                pair<Point*, Point*> opposingPair = segToPoint2.second[current->segId];
                opposing = (current->correctDir) ? opposingPair.first : opposingPair.second;
            }
            if(current->id == destination->id || (opposing != nullptr && opposing->isTraversed)){
                double totalCost = current->gCost;
                if(opposing !=nullptr){
                    totalCost += opposing->parent->gCost;
                }
                pathFoundMutex.lock();
                if(totalCost < maxCost){
                    path = current;
                    if(opposing !=nullptr){
                        path2 = opposing->parent;
                    }
                    maxCost = totalCost;
                }
                pathFoundMutex.unlock();
                if(bidirectionalEasy)
                    break;
                continue;
            }

            vector<Point*> neighbours= getNeighbours(current, turn_penalty, isOpposite);
            
            for(Point* neighbour: neighbours){
                bool isGCostHigh = neighbour->gCost >= maxInterGCost[neighbour->id];
                if(!isGCostHigh){
                    double newMaxInterGCost = neighbour->gCost;
                    if(fullyOptimal){
                        newMaxInterGCost += turn_penalty;
                    }
                    if(newMaxInterGCost < maxInterGCost[neighbour->id]){
                        maxInterGCost[current->id] = maxInterGCost[neighbour->id];
                    }
                    Point** neighbourInOpen = &(neighbour->correctDir? segToPoint.second[neighbour->segId].first:segToPoint.second[neighbour->segId].second);
                    bool hasBeenTraversed = *neighbourInOpen != nullptr && (*neighbourInOpen)->isTraversed; // closed does not contain current segment
                    bool isBetter = *neighbourInOpen == nullptr || neighbour->gCost < (*neighbourInOpen)->gCost;
                    if(!hasBeenTraversed && isBetter){
                        neighbour->updateFCost();
                        if(neighbour->fCost < maxCost){
                            if(*neighbourInOpen == nullptr){
                                //clock_t t =clock();
                                open.add(neighbour);
                                segToPoint.first.lock();
                                *neighbourInOpen = neighbour;
                                segToPoint.first.unlock();
                                //totalTime +=(clock() -t);
                                continue;
                            }
                            else{
                                (*neighbourInOpen)->copy(*neighbour);
                                open.updateItem(*neighbourInOpen);
                            }
                        }
                    }
                }
                delete neighbour;
            }
        }
    }
    //cout << "done start: " <<totalTime/(double)CLOCKS_PER_SEC<< endl;
    pathFoundMutex.lock();
    pathFound = true;
    pathFoundMutex.unlock();
}


vector<Point*> PathFinder::getNeighbours(Point* current, double turn_penalty, bool isOpposite){
    vector<StreetSegmentIdx> streetSegs = findStreetSegmentsOfIntersection(current->id);
    vector<Point*> neighbours;
    StreetSegmentInfo curSegInfo;
    if(current->segId >=0)
        curSegInfo = osmhelp::getStreetSegmentInfo(current->segId);
    for (int i = 0; i < streetSegs.size(); i++)
    {
        StreetSegmentInfo neigbourSegInfo = osmhelp::getStreetSegmentInfo(streetSegs[i]);
        bool isCorrectWay = (neigbourSegInfo.from == current->id) ^ isOpposite;
        if(!neigbourSegInfo.oneWay || isCorrectWay){ // check one way direction
            IntersectionIdx curIntIdx = findOppositeIntersection(current->id, streetSegs[i]);
            //newPointMutex.lock();
            Point* neighbour = new Point(curIntIdx, current->destCoords, current);
            //newPointMutex.unlock();
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


double PathFinder::getMinGCost(bool isOpposite){
    // if(path2 == nullptr && path1 == nullptr)
    //     return -1;
    // Point** minGCostPoint = isOpposite?&minGCostP2:&minGCostP1;
    // double* curGCost = isOpposite?&minGCost2:&minGCost1;
    // if(!(*minGCostPoint)->isTraversed && *curGCost == (*minGCostPoint)->gCost)
    //     return *curGCost;
    // Heap curHeap = isOpposite?open2:open1;
    // double newGCost = DBL_MAX;
    // Point* newPoint;
    // for(Point* item : curHeap.items){
    //     if(item->gCost < newGCost){
    //         newGCost= item->gCost;
    //         newPoint= item;
    //     }
    // }
    // *minGCostPoint = newPoint;
    // *curGCost = newGCost;
    // return newGCost;
}