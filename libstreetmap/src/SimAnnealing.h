#pragma once
#include <vector>
#include "CourierProblemInfo.h"
#include <atomic>
using namespace std;

vector<int> neighbourSolution(vector<int> route, const CourierProblemInfo& cPI);
vector<int> simulatedAnnealing(vector<int> &bestRoute
, const CourierProblemInfo& cPI, atomic<bool> &forceQuit) ;
