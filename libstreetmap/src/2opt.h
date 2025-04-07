#pragma once
#include "m2.h"
#include "ezgl/point.hpp"
#include "stdio.h"
#include <stdlib.h>
using namespace std;


void twooptSwap(vector<IntersectionIdx>& path, int indexA, int indexB); //helper function to reverse an array from [0] to [indexB] as 2 discrete elements around indexA
void optimize2Opt(vector<IntersectionIdx>& path, double turnPenalty); //optimizes closed path using 2op algorithm
double getTravelTime(vector<IntersectionIdx>& path, double turnPenalty); //find travel time that goes through a set of arbitrary paths (will use k-clustering once implemented)
void randomIntersections(vector<IntersectionIdx>& toAdd, int size); //helper function to return a list of random intersections
void optimize3Opt(vector<IntersectionIdx>& path, double turnPenalty);
vector<IntersectionIdx> threeoptSwap(vector<IntersectionIdx>& path, int indexA, int indexB, int indexC);

void optimal3EdgeReconstruction(int firstEdgeIndex, int secondEdgeIndex, int thirdEdgeIndex, int firstEdgeDestination, int secondEdgeDestination, int thirdEdgeDestination, vector<IntersectionIdx> path, double turnPenalty, double& bestTime, vector<IntersectionIdx>& bestRoute); //helper function for multithreading 3opt
void basic2NodeReconstruction(int firstNode, int secondNode, vector<IntersectionIdx>& path, double turnPenalty, double& oldTime, double& improvement); //helper function for multithreading 2opt
void setForceQuit(bool _forceQuit); //setter function to set force quit and disable all 2opt/3opt operations at the next eligible instance