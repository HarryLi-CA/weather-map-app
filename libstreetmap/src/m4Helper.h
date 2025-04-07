#pragma once
#include <vector>
#include <utility>
#include <math.h>
#include <thread>
#include "CourierProblemInfo.h"

double distance(const IntersectionIdx &a, const IntersectionIdx &b);
std::pair<IntersectionIdx, double> closestDepot(IntersectionIdx intIdx,const CourierProblemInfo &cPI, bool isStart);
void limitThreadCount(std::vector<std::thread>& threadContainer, int limit); //helper function to get current number of active threads