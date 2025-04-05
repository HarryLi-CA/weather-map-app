#pragma once
#include "2opt.h"
#include "m2.h"
#include "m3.h"
#include "m4.h"
#include "m4Helper.h"
#include "ezgl/point.hpp"
#include "stdio.h"
#include "OSMAPIHelper.h"
#include <stdlib.h>
#include "time.h"
#include <thread>
#include <mutex>
#include <map>

using namespace std;

std::mutex mutexFlag;
bool forceQuit = false;

vector<int> twooptSwap(vector<int>& path, int indexA, int indexB)
{
    vector<int> temp = path;
    indexA++;
    while(indexA < indexB)
    {
        //swap values at index A and index B
        IntersectionIdx tempIntersection = temp[indexA];
        temp[indexA] = temp[indexB];
        temp[indexB] = tempIntersection;
        //update indices for next iteration
        indexA++;
        indexB--;
    }
    return temp;
}

int activeThreadCount(vector<thread>& threads)
{
    int threadCount = 0;
    for(int i = 0; i < threads.size(); i++)
    {
        if(!threads[i].joinable())
        {
            threadCount++;
        }
    }
    return threadCount;
}

vector<int> threeoptSwap(vector<int>& path, int indexA, int indexB, int indexC, int Ato, int Bto, int Cto)
{
    vector<int> temp = twooptSwap(path, indexA, Ato);
    temp = twooptSwap(temp, indexB, Bto);
    temp = twooptSwap(temp, indexC, Cto);
    return temp;
}

map<pair<IntersectionIdx,IntersectionIdx>, double> timeCache;

double getTravelTime(vector<int>& path, CourierProblemInfo problemInfo)
{
    double totalTime = 0;
    int n = (*problemInfo.deliveries).size();
    //find fastest route between every point
    for(int i = 0; i < path.size() - 1; i++)
    {
        mutexFlag.lock();
        DeliveryInf dFrom = (*problemInfo.deliveries)[path[i]%n];
        DeliveryInf dTo = (*problemInfo.deliveries)[path[i+1]%n];
        mutexFlag.unlock();
        IntersectionIdx from = (path[i] < n) ? dFrom.pickUp : dFrom.dropOff;
        IntersectionIdx to = (path[i+1] < n) ? dTo.pickUp : dTo.dropOff;

        //add time to closest depot at start and end
        if(i == 0)
        {
            double closestTime = closestDepot(from,  problemInfo, true).second;
            totalTime += closestTime;
        }
        if(i == path.size() - 2)
        {
            double closestTime = closestDepot(to,  problemInfo, false).second;
            totalTime += closestTime;
        }
        

        if(from == to){continue;}
        mutexFlag.lock();
        double fastestTime = timeCache[{from, to}];
        mutexFlag.unlock();
        if(fastestTime == 0){
            fastestTime = computePathTravelTime(problemInfo.turn_penalty, findPathBetweenIntersections(problemInfo.turn_penalty, {from, to}));
            mutexFlag.lock();
            timeCache[{from, to}] = fastestTime;
            mutexFlag.unlock();
        }
        totalTime += fastestTime;
    }
    return totalTime;
}

void optimal3EdgeReconstruction(int firstEdgeIndex, int secondEdgeIndex, int thirdEdgeIndex, int firstEdgeDestination, int secondEdgeDestination, int thirdEdgeDestination, vector<IntersectionIdx>& path, CourierProblemInfo problemInfo, double& bestTime, vector<IntersectionIdx>& bestRoute)
{
    mutexFlag.lock();
    vector<IntersectionIdx> pathCopy = path;
    int n = problemInfo.deliveries->size();
    mutexFlag.unlock();
    vector<IntersectionIdx> newPath = threeoptSwap(pathCopy, firstEdgeIndex, secondEdgeIndex, thirdEdgeIndex, firstEdgeDestination, secondEdgeDestination, thirdEdgeDestination);
    if(!validPath(newPath, problemInfo)){return;}
    for(int i = 0; i <= newPath.size(); i++)
    {
        //to test all starts, shift the vector by one
        if(i > 0)
        {
            int temp = newPath[0];
            for(int i = 0; i < newPath.size() - 1; i++)
            {
                newPath[i] = newPath[i + 1];
            }
            newPath[newPath.size() - 1] = temp;
        }
        if(newPath[0] >= n){continue;}
        double newTime = getTravelTime(newPath, problemInfo);
        mutexFlag.lock();
        if(newTime < bestTime)
        {
            bestTime = newTime;
            bestRoute = newPath;
        }
        mutexFlag.unlock();
    }
}

void basic2NodeReconstruction(int firstNode, int secondNode, vector<IntersectionIdx>& path, CourierProblemInfo problemInfo,  double& oldTime, double& improvement)
{
    mutexFlag.lock();
    vector<IntersectionIdx> pathCopy = path;
    mutexFlag.unlock();
    vector<IntersectionIdx> newPath = twooptSwap(pathCopy, firstNode, secondNode);
    if(!validPath(newPath, problemInfo)){return;}
    for(int i = 0; i <= newPath.size(); i++)
    {
        //to test all starts, shift the vector by one
        if(i > 0)
        {
            int temp = newPath[0];
            for(int i = 0; i < newPath.size() - 1; i++)
            {
                newPath[i] = newPath[i + 1];
            }
            newPath[newPath.size() - 1] = temp;
        }
        if(newPath[0] >= n){continue;}
        double newTime = getTravelTime(newPath, problemInfo);
        mutexFlag.lock();
        double localImprovement =  oldTime - newTime;
        if(localImprovement > improvement)
        {
            path = newPath; //set path to better path
            oldTime = newTime; //set time of old best path to time of new best path
            improvement = localImprovement;
        }
        mutexFlag.unlock();
    }
}

bool validPath(vector<IntersectionIdx>& path,CourierProblemInfo problemInfo)
{
    //assume (*problemInfo.deliveries) are in order of pickup
    int deliveryIndex = 0;
    map<int, bool> seen;
    for(int i = 0; i < path.size(); i++)
    {
        if(path[i] > (*problemInfo.deliveries).size() && seen[path[i] - (*problemInfo.deliveries).size()] == false)
        {
            return false;
        }
        seen[path[i]] = true;
    }
    return true;
} 

void optimize3Opt(vector<int>& path,  CourierProblemInfo problemInfo)
{
    double improvement = DBL_MAX;
    double oldTime = 0;
    int pathSize = path.size();
    while(improvement > 0)
    {
        improvement = 0;
        //go through all eligible nodes to make improvements
        double oldTime = getTravelTime(path, problemInfo);

        

        for(int firstEdgeIndex = 0; firstEdgeIndex < pathSize - 1; firstEdgeIndex++)
        {
            for(int secondEdgeIndex = firstEdgeIndex + 2; secondEdgeIndex < pathSize - 1; secondEdgeIndex++)
            {
                for(int thirdEdgeIndex = secondEdgeIndex + 2; thirdEdgeIndex < pathSize; thirdEdgeIndex++)
                {
                    //cout << "(" << firstEdgeIndex << ", " << secondEdgeIndex << ", " << thirdEdgeIndex <<")" << endl;
                    //perform 2opt swap to check if the path is better
                    if(firstEdgeIndex == secondEdgeIndex || secondEdgeIndex == thirdEdgeIndex || firstEdgeIndex == thirdEdgeIndex){continue;}

                    vector<IntersectionIdx> bestRoute;
                    double bestTime = DBL_MAX;

                    vector<thread> threads;

                    vector<int> toIndices = {firstEdgeIndex, secondEdgeIndex, thirdEdgeIndex};

                    for(int firstEdgeDestinationIndex = 0; firstEdgeDestinationIndex < 3; firstEdgeDestinationIndex++)
                    {
                        for(int secondEdgeDestinationIndex = firstEdgeDestinationIndex; secondEdgeDestinationIndex < 3; secondEdgeDestinationIndex++)
                        {
                            for(int thirdEdgeDestinationIndex = secondEdgeDestinationIndex; thirdEdgeDestinationIndex < 3; thirdEdgeDestinationIndex++)
                            {
                                if(firstEdgeDestinationIndex == 0 && secondEdgeDestinationIndex == 0 && thirdEdgeDestinationIndex == 0){continue;} //in this case the path is the same
                                while(activeThreadCount(threads) < 30);
                                thread t(optimal3EdgeReconstruction,firstEdgeIndex, secondEdgeIndex, thirdEdgeIndex, 
                                toIndices[firstEdgeDestinationIndex], toIndices[secondEdgeDestinationIndex], toIndices[thirdEdgeDestinationIndex],
                                ref(path), problemInfo, ref(bestTime), ref(bestRoute));
                                
                                threads.push_back(move(t));
                                
                                // optimal3EdgeReconstruction(firstEdgeIndex, secondEdgeIndex, thirdEdgeIndex, 
                                // toIndices[firstEdgeDestinationIndex], toIndices[secondEdgeDestinationIndex], toIndices[thirdEdgeDestinationIndex],
                                // path, turnPenalty, bestTime, bestRoute);
                            }
                        }
                    }
                    
                    for(int i = 0; i < threads.size(); i++)
                    {
                        threads[i].join();
                    }

                    if(forceQuit){break;}

                    double localImprovement =  oldTime - bestTime;
                
                    if(localImprovement > improvement)
                    {
                        path = bestRoute; //set path to better path
                        oldTime = bestTime; //set time of old best path to time of new best path
                        improvement = localImprovement;
                    }
                }
                if(forceQuit){break;}
                
            }
            if(forceQuit){break;}
        }
        cout << "improvement: " << improvement << endl;
        if(forceQuit){break;}
    }
    
}




void optimize2Opt(vector<int>& path,  CourierProblemInfo problemInfo)
{
    double improvement = DBL_MAX;
    double oldTime = 0;
    int pathSize = path.size();

    while(improvement > 0)
    {
        improvement = 0;
        //go through all eligible nodes to make improvements
        double oldTime = getTravelTime(path, problemInfo);
        vector<thread> threads;
        for(int i = 0; i < pathSize - 1; i++)
        {
            for(int j = i + 2; j < pathSize; j++)
            {
                while(activeThreadCount(threads) < 30);
                
                thread t(basic2NodeReconstruction, i, j, ref(path), problemInfo, ref(oldTime), ref(improvement));
                threads.push_back(move(t));
                if(forceQuit){break;}
            }
            if(forceQuit){break;}
        }

        for(int tIndex = 0; tIndex < threads.size(); tIndex++)
        {
            threads[tIndex].join();
        }
        cout << "improvement = " << improvement << endl;

        if(forceQuit){break;}
    }
}

void randomIntersections(vector<int>& toAdd, std::vector<DeliveryInf>& deliveries, vector<IntersectionIdx>& depots, int size)
{
    //srand(time(NULL));
    vector<IntersectionIdx> list;
    for(int i = 0; i < size; i++)
    {
        IntersectionIdx pick = rand()%osmhelp::getNumIntersections();
        IntersectionIdx drop = rand()%osmhelp::getNumIntersections();
        toAdd.push_back(i);
        toAdd.push_back(i + size);
        deliveries.push_back({pick, drop});

        depots.push_back(rand()%osmhelp::getNumIntersections());
    }
    return;
}

void setForceQuit(bool _forceQuit)
{
    forceQuit = _forceQuit;
}