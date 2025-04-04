#include "m4.h"
#include <vector>
#include "CourierProblemInfo.h"
std::vector<CourierSubPath> travelingCourier(
    const float turn_penalty,
    const std::vector<DeliveryInf>& deliveries,
    const std::vector<IntersectionIdx>& depots)
{
    CourierProblemInfo cPI{turn_penalty,&deliveries, &depots};
}