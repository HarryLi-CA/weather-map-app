#include <vector>
#include <utility>
#include <math.h>
#include "CourierProblemInfo.h"
#include "m4Helper.h"
#include <thread>

using namespace std;

//get distance between two intersections
double distance(const IntersectionIdx &a, const IntersectionIdx &b) {
    return -1;
}

pair<IntersectionIdx, double> closestDepot(IntersectionIdx intIdx,const CourierProblemInfo &cPI, bool isStart){
    double minTime = __DBL_MAX__;
    IntersectionIdx closeDepot = -1;
    int nDepots = cPI.depots->size();
    for (int i = 0; i < nDepots; i++)
    {
        IntersectionIdx curDepot = (*cPI.depots)[i];
        double curDist = isStart ? distance(intIdx,curDepot) :  distance(curDepot, intIdx);
        if(curDist < minTime){
            minTime = curDist;
            closeDepot = curDepot;
        }
    }
    return make_pair(closeDepot, minTime);
}


void limitThreadCount(vector<std::thread>& threadContainer, int limit)
{
    while(threadContainer.size() >= limit)
    {
        threadContainer[0].join();
        threadContainer.erase(threadContainer.begin());
    }
}