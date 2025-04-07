#include <vector>
#include <utility>
#include <math.h>
#include "CourierProblemInfo.h"
#include "m4Helper.h"
#include "2opt.h"
#include <thread>
#include <boost/functional/hash.hpp>

#include <unordered_map>

using namespace std;

unordered_map <pair<IntersectionIdx, bool>, pair<IntersectionIdx, double> , boost::hash<pair<IntersectionIdx, bool>>> closestDepotCache;

//get distance between two intersections
double distance(const IntersectionIdx &a, const IntersectionIdx &b) {
    return -1;
}

pair<IntersectionIdx, double> closestDepot(IntersectionIdx intIdx,const CourierProblemInfo &cPI, bool isStart){
    auto searchPair = closestDepotCache.find(make_pair(intIdx,isStart));
    if(searchPair != closestDepotCache.end()){
        pair<IntersectionIdx, double> result = searchPair->second;
        return result;
    }
    double minTime = __DBL_MAX__;
    IntersectionIdx closeDepot = -1;
    int nDepots = cPI.depots->size();
    for (int i = 0; i < nDepots; i++)
    {
        IntersectionIdx curDepot = (*cPI.depots)[i];
        double curDist = isStart ? findDistanceBetweenTwoPoints(osmhelp::getIntersectionPosition(intIdx), osmhelp::getIntersectionPosition(curDepot)):  findDistanceBetweenTwoPoints(osmhelp::getIntersectionPosition(curDepot), osmhelp::getIntersectionPosition(intIdx));
        if(curDist < minTime){
            minTime = curDist;
            closeDepot = curDepot;
        }
    }
    pair<IntersectionIdx, double> result = make_pair(closeDepot, minTime);
    closestDepotCache[make_pair(intIdx,isStart)] = result;
    return result;
}


void limitThreadCount(vector<std::thread>& threadContainer, int limit)
{
    while(threadContainer.size() >= limit)
    {
        threadContainer[0].join();
        threadContainer.erase(threadContainer.begin());
    }
}