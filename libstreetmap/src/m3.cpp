#include "ezgl/graphics.hpp"
#include "ezgl/canvas.hpp"
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include <vector>
using namespace std;

void drawPath(ezgl::renderer *g, vector<StreetSegmentIdx> path)
{
    g->set_line_width(5);
    for(int pathIndex = 0; pathIndex < path.size(); pathIndex++)
    {
        StreetSegmentIdx segmentIndex = path[pathIndex];
    }
}