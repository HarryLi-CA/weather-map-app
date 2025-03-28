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

//static clock_t totalTime = 0;
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
    intersectionsVisited1.resize(osmhelp::getNumIntersections(), nullptr);
    intersectionsVisited2.resize(osmhelp::getNumIntersections(), nullptr);
    segToPointCorrect.second.resize(osmhelp::getNumStreetSegments(), make_pair(nullptr, nullptr));
    segToPointOpposite.second.resize(osmhelp::getNumStreetSegments(), make_pair(nullptr, nullptr));
    maxInterGCostCorrect.resize(osmhelp::getNumIntersections(), DBL_MAX);
    maxInterGCost2.resize(osmhelp::getNumIntersections(), DBL_MAX);
    //mSTCorrect = new MinSegmentTree(osmhelp::getNumStreetSegments()*2);
    //mSTOpposite = new MinSegmentTree(osmhelp::getNumStreetSegments()*2);
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
    for(int i =0; i < open1.currentItemCount; i++)
        delete open1.items[i];
    for(int i =0; i < open2.currentItemCount; i++)
        delete open2.items[i];
    //cout << "done delete: " <<(clock() -t)/(double)CLOCKS_PER_SEC << endl;
    
    delete mSTCorrect;
    delete mSTOpposite;
}

vector<Point*> PathFinder::calculatePathArray(const double turn_penalty, const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids){
    //cout << "searching for path "<<intersect_ids.first << " " << intersect_ids.second <<'\n';
    //clock_t t = clock();
    //get paths from both ends simultaneously
    thread t1(&PathFinder::findHalfPath, this, turn_penalty, intersect_ids.first, intersect_ids.second, 
        ref(open1), ref(closed1), ref(segToPointCorrect),
        ref(maxInterGCostCorrect), ref(segToPointOpposite),
        ref(pathCorrect), ref(pathOpposite), ref(intersectionsVisited1), ref(intersectionsVisited2),  false);
    thread t2(&PathFinder::findHalfPath, this, turn_penalty, intersect_ids.second, intersect_ids.first, 
        ref(open2), ref(closed2), ref(segToPointOpposite),
        ref(maxInterGCost2), ref(segToPointCorrect),
        ref(pathOpposite), ref(pathCorrect),ref(intersectionsVisited2), ref(intersectionsVisited1), true);
    t1.join();
    t2.join();
    //totalTime +=(clock() -t);
    //cout << "path found: " <<totalTime/(double)CLOCKS_PER_SEC<< endl;
    //cout << "got path" << endl;
    vector<Point*> out = {};
    if(pathCorrect == nullptr && pathOpposite == nullptr)
        cout << "Error: Path Not Found\n";
    if(pathCorrect !=nullptr)
        out = pathCorrect->retracePath(out, false);
    //cout << "path1 complete" << endl;
    if(pathOpposite !=nullptr)
        out = pathOpposite->retracePath(out, true);
    //cout << "path2 complete" << endl;
    return out;
}

void PathFinder::findHalfPath(const double turn_penalty, IntersectionIdx begin, IntersectionIdx end, 
    Heap& open, vector<Point*>& closed, pair<mutex,vector<pair<Point*, Point*>>>& segToPoint,
    vector<double>& maxInterGCost, pair<mutex,vector<pair<Point*, Point*>>>& segToPoint2,
    Point*& path, Point*& path2, vector<Point*>& intersectionsVisited, vector<Point*>& opIntersectionsVisited, const bool isOpposite){
    
    Point* start = new Point(begin, osmhelp::getPoint2dOfIntersections(end), nullptr);

    Point* current = start;

    start->gCost = 0;
    open.add(start);

    //cout<<"Ended basic initial with seconds: "<< (double)(clock() - startTime) / CLOCKS_PER_SEC << endl;

    while(open.length() > 0){
        closed.push_back(open.removeFirst());
        current = closed.back();
        current->isTraversed = true;

        if(current->fCost>=maxCost)
            break;

        // double oppositeMin = getMinGCost(!isOpposite);
        // if(oppositeMin >= 0 && current->gCost + oppositeMin > maxCost)
        //     continue;

        if(optimalPathFound){
            return;
        }

        //MinSegmentTree* mST1 = isOpposite?mSTOpposite:mSTCorrect;
        //MinSegmentTree* mST2 = isOpposite?mSTCorrect:mSTOpposite;
        Point** minGCostP1 = isOpposite?&minGCostPOpposite:&minGCostPCorrect;
        Point** minGCostP2 = isOpposite?&minGCostPCorrect:&minGCostPOpposite;

        if(*minGCostP2 !=nullptr && (current->gCost + (*minGCostP2)->gCost >= maxCost))
            continue;

        if(*minGCostP1 != nullptr){
            //cout<<"test2"<<endl;
            //mST1->update(1, 0, mST1->maxArrSize-1, current->segTreeIndex, DBL_MAX);
            if(current == *minGCostP1){
                for(int i = 0; i < open.currentItemCount;i++){
                    if(open.items[i]->gCost < (*minGCostP1)->gCost)
                         *minGCostP1 = open.items[i];
                }
            }
        }
        if(*minGCostP1 ==nullptr && open.currentItemCount>0 && possiblePathFound){
            //cout<<"test" << endl;
            for(int i = 0; i < open.currentItemCount;i++){
                if((*minGCostP1) == nullptr || open.items[i]->gCost < (*minGCostP1)->gCost)
                     *minGCostP1 = open.items[i];
            }
        }

        if(current->gCost < maxInterGCost[current->id]){
            Point* opposing = nullptr;
            if(current->segId >= 0){
                pair<Point*, Point*> opposingPair = segToPoint2.second[current->segId];
                opposing = (current->correctDir) ? opposingPair.first : opposingPair.second;
            }
            if(current->id == end || (opposing != nullptr && opposing->isTraversed)){
                double totalCost = current->gCost;
                if(opposing !=nullptr){
                    totalCost += opposing->parent->gCost;
                }
                possiblePathFound = true;
                if(totalCost < maxCost){
                    path = current;
                    if(opposing !=nullptr){
                        path2 = opposing->parent;
                    }
                    maxCost = totalCost;
                }
                if(bidirectionalEasy)
                    break;
                
                continue;
            }
            vector<StreetSegmentIdx>* streetSegs = findStreetSegmentsOfIntersectionPtr(current->id);
            StreetSegmentInfo curSegInfo;
            if(current->segId >=0)
                curSegInfo = osmhelp::getStreetSegmentInfo(current->segId);
            
            for(StreetSegmentIdx streetSeg: (*streetSegs)){
                StreetSegmentInfo* neigbourSegInfo = osmhelp::getStreetSegmentInfoPtr(streetSeg);
                bool isCorrectWay = (neigbourSegInfo->from == current->id) ^ isOpposite;
                if(!neigbourSegInfo->oneWay || isCorrectWay){ // check one way direction
                    
                    IntersectionIdx curIntIdx = neigbourSegInfo->from == current->id?neigbourSegInfo->to : neigbourSegInfo->from;
                    //newPointMutex.lock();
                    //newPointMutex.unlock();
                    int neighbourSegId = streetSeg;

                    double pathTravelTime = 0;
                    if(current->parent != nullptr){
                        //if streets of segments are not the same, there is a turn
                        if(neigbourSegInfo->streetID != curSegInfo.streetID)
                        {
                            pathTravelTime += turn_penalty; //apply turn penalty
                        }
                    }

                    double neighbourGCost = current->gCost + findStreetSegmentTravelTime(neighbourSegId) + pathTravelTime;
                    bool isGCostHigh = neighbourGCost >= maxInterGCost[curIntIdx] || (*minGCostP2 !=nullptr && (current->gCost + (*minGCostP2)->gCost >= maxCost));
                    if(!isGCostHigh){
                        double newMaxInterGCost = neighbourGCost;
                        if(fullyOptimal){
                            newMaxInterGCost += turn_penalty;
                        }
                        if(newMaxInterGCost < maxInterGCost[curIntIdx]){
                            maxInterGCost[current->id] = maxInterGCost[curIntIdx];
                        }
                        Point** neighbourInOpen; 
                        if(fullyOptimal)
                            neighbourInOpen= &(isCorrectWay? segToPoint.second[neighbourSegId].first:segToPoint.second[neighbourSegId].second);
                        else
                            neighbourInOpen = &(intersectionsVisited[curIntIdx]);
                        bool isBetter = *neighbourInOpen == nullptr || neighbourGCost < (*neighbourInOpen)->gCost;
                        if(isBetter){
                            double neighbourHCost =Point::calculateHCost(osmhelp::getPoint2dOfIntersections(curIntIdx),current->destCoords);
                            double neighbourFCost = neighbourGCost + neighbourHCost;
                            if(neighbourFCost < maxCost){
                                if(*neighbourInOpen == nullptr){
                                    //clock_t t =clock();
                                    Point* neighbour = new Point(curIntIdx, current->destCoords, current);
                                    neighbour->fCost = neighbourFCost;
                                    neighbour->hCost = neighbourHCost;
                                    neighbour->gCost = neighbourGCost;
                                    neighbour->segId = neighbourSegId;
                                    neighbour->correctDir = isCorrectWay;
                                    open.add(neighbour);
                                    *neighbourInOpen = neighbour;
                                    if(*minGCostP1 != nullptr &&neighbour->gCost < (*minGCostP1)->gCost){
                                        *minGCostP1 = neighbour;
                                    }
                                    continue;
                                }
                                else{
                                    (*neighbourInOpen)->fCost = neighbourFCost;
                                    (*neighbourInOpen)->gCost = neighbourGCost;
                                    (*neighbourInOpen)->segId = neighbourSegId;
                                    (*neighbourInOpen)->correctDir = isCorrectWay;
                                    (*neighbourInOpen)->parent = current;

                                    open.updateItem(*neighbourInOpen);
                                    if(*minGCostP1 !=nullptr && (*neighbourInOpen)->gCost < (*minGCostP1)->gCost){
                                        
                            *minGCostP1 = (*neighbourInOpen);
                                        //cout<<"test3"<<endl;
                                        // mST1->update(neighbour->segTreeIndex,neighbour->gCost);
                                        // if(neighbour->gCost <minGCostP1->gCost){
                                        //     minGCostP1 = mST1->query().second;
                                        // }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    pathFoundMutex.lock();
    optimalPathFound = true;
    pathFoundMutex.unlock();
}


vector<Point*> PathFinder::getNeighbours(Point* current, double turn_penalty, bool isOpposite){
    vector<StreetSegmentIdx>* streetSegs = findStreetSegmentsOfIntersectionPtr(current->id);
    int streetSegsSize = streetSegs->size();
    vector<Point*> neighbours;
    neighbours.reserve(streetSegsSize);
    StreetSegmentInfo curSegInfo;
    if(current->segId >=0)
        curSegInfo = osmhelp::getStreetSegmentInfo(current->segId);
    for (int i = 0; i < streetSegsSize; i++)
    {
        StreetSegmentInfo* neigbourSegInfo = osmhelp::getStreetSegmentInfoPtr((*streetSegs)[i]);
        bool isCorrectWay = (neigbourSegInfo->from == current->id) ^ isOpposite;
        if(!neigbourSegInfo->oneWay || isCorrectWay){ // check one way direction
            
            IntersectionIdx curIntIdx = neigbourSegInfo->from == current->id?neigbourSegInfo->to : neigbourSegInfo->from;
            //newPointMutex.lock();
            Point* neighbour = new Point(curIntIdx, current->destCoords, current);
            //newPointMutex.unlock();
            neighbour->segId = (*streetSegs)[i];
            neighbour->correctDir = isCorrectWay;

            double pathTravelTime = 0;
            if(current->parent != nullptr){
                //if streets of segments are not the same, there is a turn
                if(neigbourSegInfo->streetID != curSegInfo.streetID)
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
    return isOpposite?0.0:1.0;
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