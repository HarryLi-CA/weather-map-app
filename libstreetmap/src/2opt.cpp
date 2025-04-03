#pragma once
#include "m2.h"
#include "m3.h"
#include "ezgl/point.hpp"
#include "stdio.h"
#include "OSMAPIHelper.h"
#include <stdlib.h>
#include "time.h"
using namespace std;


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

double getTravelTime(vector<IntersectionIdx>& path, double turnPenalty)
{
    double totalTime = 0;
    //find fastest route between every point
    for(int i = 0; i < path.size() - 1; i++)
    {
        IntersectionIdx from = path[i];
        IntersectionIdx to = path[i+1];
        double fastestTime = computePathTravelTime(turnPenalty, findPathBetweenIntersections(turnPenalty, {from, to}));
        totalTime += fastestTime;
    }
    return totalTime;
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
        for(int i = 0; i < pathSize - 1; i++)
        {
            for(int j = i + 2; j < pathSize - 1; j++)
            {
                for(int k = j + 2; k < pathSize; k++)
                {
                    cout << "(" << i << ", " << j << ", " << k <<")" << endl;
                    //perform 2opt swap to check if the path is better
                    if(i == j || j == k || i == k){continue;}

                    vector<IntersectionIdx> bestRoute;
                    double bestTime = DBL_MAX;

                    vector<int> toIndices = {i, j, k};

                    for(int a = 0; a < 3; a++)
                    {
                        for(int b = a; b < 3; b++)
                        {
                            for(int c = b; c < 3; c++)
                            {
                                if(a == 0 && b == 0 && c == 0){continue;} //in this case the path is the same
                                vector<IntersectionIdx> newPath = threeoptSwap(path, i, j, k, toIndices[a],toIndices[b],toIndices[c]);
                                double newTime = getTravelTime(newPath, turnPenalty);
                                if(newTime < bestTime)
                                {
                                    bestTime = newTime;
                                    bestRoute = newPath;
                                }
                            }
                        }
                    }
                    

                    double localImprovement =  oldTime - bestTime;
                
                    if(localImprovement > improvement)
                    {
                        path = bestRoute; //set path to better path
                        oldTime = bestTime; //set time of old best path to time of new best path
                        improvement = localImprovement;
                    }
                }
                
            }
        }
        cout << "improvement: " << improvement << endl;
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
        for(int i = 0; i < pathSize - 1; i++)
        {
            for(int j = i + 2; j < pathSize; j++)
            {
                //perform 2opt swap to check if the path is better
                vector<IntersectionIdx> newPath = twooptSwap(path, i, j);

                double newTime = getTravelTime(newPath, turnPenalty);

                double localImprovement =  oldTime - newTime;
                if(localImprovement > improvement)
                {
                    path = newPath; //set path to better path
                    oldTime = newTime; //set time of old best path to time of new best path
                    improvement = localImprovement;
                }
            }
        }
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