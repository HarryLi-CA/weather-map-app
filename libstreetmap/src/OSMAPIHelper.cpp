#include <thread>
#include <mutex>
#include <utility>
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "OSMAPIHelper.h"


namespace osmhelp{
    int numIntersections = -1;
    int numStreetSegments = -1;
    int numStreets = -1;
    std::vector<StreetSegmentInfo> segInfo;
    std::vector<std::pair<bool, LatLon>> intersectionPos;

    std::mutex apiMutex;
    void load(){
        int size = getNumStreetSegments();
        for(int i =0; i< size; i++){
            getStreetSegmentInfo(i);
        }
        size = getNumIntersections();
        for(int i =0; i< size; i++){
            getIntersectionPosition(i);
        }
        size = getNumStreets();
    }
    void reset(){
        numIntersections = -1;
        numStreetSegments = -1;
        numStreets = -1;
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

}