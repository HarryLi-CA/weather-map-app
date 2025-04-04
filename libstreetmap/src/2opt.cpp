#pragma once
#include "m2.h"
#include "m3.h"
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

vector<IntersectionIdx> twooptSwap(vector<IntersectionIdx>& path, int indexA, int indexB)
{
    vector<IntersectionIdx> temp = path;
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

vector<IntersectionIdx> threeoptSwap(vector<IntersectionIdx>& path, int indexA, int indexB, int indexC, int Ato, int Bto, int Cto)
{
    vector<IntersectionIdx> temp = twooptSwap(path, indexA, Ato);
    temp = twooptSwap(temp, indexB, Bto);
    temp = twooptSwap(temp, indexC, Cto);
    return temp;
}

map<pair<IntersectionIdx,IntersectionIdx>, double> timeCache;

double getTravelTime(vector<IntersectionIdx>& path, double turnPenalty)
{
    double totalTime = 0;
    //find fastest route between every point
    for(int i = 0; i < path.size() - 1; i++)
    {
        IntersectionIdx from = path[i];
        IntersectionIdx to = path[i+1];
        double fastestTime = timeCache[{from, to}];
        if(fastestTime == 0){
            fastestTime = computePathTravelTime(turnPenalty, findPathBetweenIntersections(turnPenalty, {from, to}));
            timeCache[{from, to}] = fastestTime;
        }
        totalTime += fastestTime;
    }
    return totalTime;
}

void optimal3EdgeReconstruction(int firstEdgeIndex, int secondEdgeIndex, int thirdEdgeIndex, int firstEdgeDestination, int secondEdgeDestination, int thirdEdgeDestination, vector<IntersectionIdx> path, double turnPenalty, double& bestTime, vector<IntersectionIdx>& bestRoute)
{
    mutexFlag.lock();
    vector<IntersectionIdx> pathCopy = path;
    mutexFlag.unlock();
    vector<IntersectionIdx> newPath = threeoptSwap(pathCopy, firstEdgeIndex, secondEdgeIndex, thirdEdgeIndex, firstEdgeDestination, secondEdgeDestination, thirdEdgeDestination);
    double newTime = getTravelTime(newPath, turnPenalty);
    mutexFlag.lock();
    if(newTime < bestTime)
    {
        bestTime = newTime;
        bestRoute = newPath;
    }
    mutexFlag.unlock();
}

void basic2NodeReconstruction(int firstNode, int secondNode, vector<IntersectionIdx>& path, double turnPenalty, double& oldTime, double& improvement)
{
    mutexFlag.lock();
    vector<IntersectionIdx> copyPath = path;
    mutexFlag.unlock();
    vector<IntersectionIdx> newPath = twooptSwap(path, firstNode, secondNode);
    double newTime = getTravelTime(newPath, turnPenalty);
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

void optimize3Opt(vector<IntersectionIdx>& path, double turnPenalty)
{
    double improvement = DBL_MAX;
    double oldTime = 0;
    int pathSize = path.size();
    while(improvement > 0)
    {
        improvement = 0;
        //go through all eligible nodes to make improvements
        double oldTime = getTravelTime(path, turnPenalty);

        

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
                                thread t(optimal3EdgeReconstruction,firstEdgeIndex, secondEdgeIndex, thirdEdgeIndex, 
                                toIndices[firstEdgeDestinationIndex], toIndices[secondEdgeDestinationIndex], toIndices[thirdEdgeDestinationIndex],
                                ref(path), turnPenalty, ref(bestTime), ref(bestRoute));
                                
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

void optimize2Opt(vector<IntersectionIdx>& path, double turnPenalty)
{
    double improvement = DBL_MAX;
    double oldTime = 0;
    int pathSize = path.size();
    while(improvement > 0)
    {
        improvement = 0;
        //go through all eligible nodes to make improvements
        double oldTime = getTravelTime(path, turnPenalty);
        vector<thread> threads;
        for(int i = 0; i < pathSize - 1; i++)
        {
            for(int j = i + 2; j < pathSize; j++)
            {
                //perform 2opt swap to check if the path is better
                thread t(basic2NodeReconstruction, i, j, ref(path), turnPenalty, ref(oldTime), ref(improvement));
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

void randomIntersections(vector<IntersectionIdx>& toAdd, int size)
{
    //srand(time(NULL));
    vector<IntersectionIdx> list;
    for(int i = 0; i < size; i++)
    {
        toAdd.push_back(rand()%osmhelp::getNumIntersections());
    }
    return;
}

void setForceQuit(bool _forceQuit)
{
    forceQuit = _forceQuit;
}