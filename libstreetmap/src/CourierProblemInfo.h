
#pragma once
#include "m4.h"
#include <vector>

struct CourierProblemInfo
{
    const float turn_penalty;
    const std::vector<DeliveryInf>* deliveries;
    const std::vector<IntersectionIdx>* depots;
};

