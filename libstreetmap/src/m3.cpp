#include "ezgl/graphics.hpp"
#include "ezgl/canvas.hpp"
#include "m1.h"
#include "m3.h"
#include "StreetsDatabaseAPI.h"
#include <vector>
using namespace std;

const double kMinimumTurnAngle = M_PI/6.0; //minimum turn angle for street to be considered a turn
const double kDefaultTurnPenalty = 10; //penalty imposed on a turning street

void drawPath(ezgl::renderer *g, vector<StreetSegmentIdx> path)
{
    g->set_line_width(5);
    for(int pathIndex = 0; pathIndex < path.size(); pathIndex++)
    {
        StreetSegmentIdx segmentIndex = path[pathIndex];
    }
}

double computePathTravelTime(const double turn_penalty, const std::vector<StreetSegmentIdx>& path)
{
    double pathTravelTime = 0;
    for(int pathIndex = 0; pathIndex < path.size(); pathIndex++)
    {
        StreetSegmentIdx segmentIndex = path[pathIndex];
        
        double travelTime = findStreetSegmentTravelTime(segmentIndex);
        pathTravelTime += travelTime; //add travel time

        if(pathIndex < path.size() - 1)
        {
            //get next segment
            StreetSegmentIdx nextSegmentIndex = path[pathIndex+1];
            //get info for both segments
            StreetSegmentInfo currentSegmentInfo = getStreetSegmentInfo(segmentIndex);
            StreetSegmentInfo nextSegmentInfo = getStreetSegmentInfo(nextSegmentIndex);
            //if streets of segments are not the same, there is a turn
            if(currentSegmentInfo.streetID != nextSegmentInfo.streetID)
            {
                pathTravelTime += turn_penalty; //apply turn penalty
            }
        }
    }
    return pathTravelTime;
}