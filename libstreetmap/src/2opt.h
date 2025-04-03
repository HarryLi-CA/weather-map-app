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