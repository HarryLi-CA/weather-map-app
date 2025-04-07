#pragma once
#include "m2.h"
#include "m4.h"
#include "m4Helper.h"
#include "ezgl/point.hpp"
#include "stdio.h"
#include <stdlib.h>
#include "CourierProblemInfo.h"

using namespace std;

vector<int>  twooptSwap(vector<int>& path, int indexA, int indexB); //helper function to reverse an array from [0] to [indexB] as 2 discrete elements around indexA
void optimize2Opt(vector<int>& path, CourierProblemInfo problemInfo); //optimizes closed path using 2op algorithm
double getTravelTime(vector<int>& path, CourierProblemInfo problemInfo); //find travel time that goes through a set of arbitrary paths (will use k-clustering once implemented)
double getCheapTravelTime(vector<int>& path, CourierProblemInfo problemInfo); //find rough estimation of travel time 
void randomIntersections(vector<int>& toAdd, std::vector<DeliveryInf>& deliveries, vector<IntersectionIdx>& depots, int size); //helper function to return a list of random intersections
void optimize3Opt(vector<int>& path, CourierProblemInfo problemInfo);
vector<int> threeoptSwap(vector<int>& path, int indexA, int indexB, int indexC);

bool validPath(vector<IntersectionIdx>& path,CourierProblemInfo problemInfo); //helper function to check if a path is valid

void optimal3EdgeReconstruction(int firstEdgeIndex, int secondEdgeIndex, int thirdEdgeIndex, int firstEdgeDestination, int secondEdgeDestination, int thirdEdgeDestination, vector<IntersectionIdx>& path, CourierProblemInfo problemInfo, double& bestTime, vector<IntersectionIdx>& bestRoute); //helper function for multithreading 3opt
void basic2NodeReconstruction(int firstNode, int secondNode, vector<IntersectionIdx>& path, CourierProblemInfo problemInfo, double& oldTime, double& improvement); //helper function for multithreading 2opt

void setForceQuit(bool _forceQuit); //setter function to set force quit and disable all 2opt/3opt operations at the next eligible instance
bool getDone(); //getter function to check whether 2opt/3opt operations are complete
void setDone(bool _done); //setter function to set if 2opt/3opt operations are complete

vector<StreetSegmentIdx> cachedPath(IntersectionIdx from, IntersectionIdx to, CourierProblemInfo problemInfo); //helper function to return previously calculated paths