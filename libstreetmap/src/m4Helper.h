#pragma once
#include <vector>
#include <utility>
#include <math.h>
#include "CourierProblemInfo.h"

double distance(const IntersectionIdx &a, const IntersectionIdx &b);
std::pair<IntersectionIdx, double> closestDepot(IntersectionIdx intIdx,const CourierProblemInfo &cPI, bool isStart);