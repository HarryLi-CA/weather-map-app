#include <thread>
#include <mutex>
#include <utility>
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "OSMAPIHelper.h"
#include <math.h>

#include "ezgl/application.hpp"
#include "pointMath.h"
#include "renderHelper.h"


namespace osmhelp{
    int numIntersections = -1;
    int numStreetSegments = -1;
    int numStreets = -1;
    double maxSpeedLimit = -1;
    double minSpeedLimit = DBL_MAX;
    std::vector<StreetSegmentInfo> segInfo;
    std::vector<std::pair<bool, LatLon>> intersectionPos;
    std::vector<std::pair<bool, ezgl::point2d>> intersectionCoord;

    std::mutex apiMutex;
    void load(){
        int size = getNumStreetSegments();
        for(int i =0; i< size; i++){
            getStreetSegmentInfo(i);
        }
        size = getNumIntersections();
        for(int i =0; i< size; i++){
            getIntersectionPosition(i);
            getPoint2dOfIntersections(i);
        }
        size = getNumStreets();
        getSpeedLimitMaximum();
        //getSpeedLimitMinimum();
    }
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
            intersectionCoord.resize(numIntersections, std::make_pair(false,ezgl::point2d(0,0)));
        }
        return numIntersections;
    }

    int getNumStreetSegments(){
        if(numStreetSegments < 0){
            apiMutex.lock();
            numStreetSegments = ::getNumStreetSegments();
            apiMutex.unlock();

            StreetSegmentInfo newSegInfo = {OSMID(-1), -1, -1, false, -1, -1, -1};
            segInfo.resize(numStreetSegments,newSegInfo);
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
    ezgl::point2d getPoint2dOfIntersections(const IntersectionIdx idx){
        if(idx >= getNumIntersections()){ //ensure vector is sized
            throw idx;
        }
        if(!intersectionPos[idx].first){
            apiMutex.lock();
            intersectionCoord[idx].second = LatLonToPoint2D(osmhelp::getIntersectionPosition(idx));
            apiMutex.unlock();
            intersectionCoord[idx].first = true;
        }
        return intersectionCoord[idx].second;
    }
    StreetSegmentInfo getStreetSegmentInfo(const StreetSegmentIdx idx){
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
    StreetSegmentInfo* getStreetSegmentInfoPtr(const StreetSegmentIdx idx){
        return &(segInfo[idx]);
    }

    LatLon getIntersectionPosition(const IntersectionIdx idx){
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
    double getSpeedLimitMinimum()
    {
        if(minSpeedLimit == DBL_MAX) //if max speed limit is default value, it needs to be calculated
        {
            apiMutex.lock();
            for(StreetSegmentIdx segIndex = 0; segIndex < getNumStreetSegments(); segIndex++)
            {
                minSpeedLimit = std::min(minSpeedLimit, (double)getStreetSegmentInfo(segIndex).speedLimit);
            }
            apiMutex.unlock();
        }

        return minSpeedLimit;
    }

}