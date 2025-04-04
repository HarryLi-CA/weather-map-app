#include "m4.h"
#include <vector>

struct CourierProblemInfo
{
    const float turn_penalty;
    const std::vector<DeliveryInf>* deliveries;
    conststd::vector<IntersectionIdx>* depots;
};

