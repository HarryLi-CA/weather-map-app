#include <thread>
#include <mutex>
#include <utility>
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "OSMAPIHelper.h"
#include <math.h>


namespace osmhelp{
    int numIntersections = -1;
    int numStreetSegments = -1;
    int numStreets = -1;
    double maxSpeedLimit = -1;
    std::vector<StreetSegmentInfo> segInfo;
    std::vector<std::pair<bool, LatLon>> intersectionPos;

    std::mutex apiMutex;

    void reset(){
        numIntersections = -1;
        numStreetSegments = -1;
        numStreets = -1;
        maxSpeedLimit = -1;
        segInfo.clear();
        intersectionPos.clear();
    }

    int getNumIntersections(){
        if(numIntersections < 0){
            apiMutex.lock();
            numIntersections = ::getNumIntersections();
            apiMutex.unlock();

            intersectionPos.resize(numIntersections, std::make_pair(false,LatLon(0,0)));
        }
        return numIntersections;
    }

    int getNumStreetSegments(){
        if(numStreetSegments < 0){
            apiMutex.lock();
            numStreetSegments = ::getNumStreetSegments();
            apiMutex.unlock();

            segInfo.resize(numStreetSegments,{.speedLimit = -1});
        }
        return numStreetSegments;
    }

    int getNumStreets(){
        if(numStreets < 0){
            apiMutex.lock();
            numStreets = ::getNumStreets();
            apiMutex.unlock();
        }
        return numStreets;
    }

    StreetSegmentInfo getStreetSegmentInfo(StreetSegmentIdx idx){
        if(idx >= getNumStreetSegments()){ //ensure vector is sized
            throw idx;
        }
        if(segInfo[idx].speedLimit <= 0){
            apiMutex.lock();
            segInfo[idx] = ::getStreetSegmentInfo(idx);
            apiMutex.unlock();
        }
        return segInfo[idx];
    }

    LatLon getIntersectionPosition(IntersectionIdx idx){
        if(idx >= getNumIntersections()){ //ensure vector is sized
            throw idx;
        }
        if(!intersectionPos[idx].first){
            apiMutex.lock();
            intersectionPos[idx].second = ::getIntersectionPosition(idx);
            apiMutex.unlock();
            intersectionPos[idx].first = true;
        }
        return intersectionPos[idx].second;
    }

    double getSpeedLimitMaximum()
    {
        if(maxSpeedLimit == -1) //if max speed limit is default value, it needs to be calculated
        {
            apiMutex.lock();
            for(StreetSegmentIdx segIndex = 0; segIndex < getNumStreetSegments(); segIndex++)
            {
                maxSpeedLimit = std::max(maxSpeedLimit, (double)getStreetSegmentInfo(segIndex).speedLimit);
            }
            apiMutex.unlock();
        }

        return maxSpeedLimit;
    }

}