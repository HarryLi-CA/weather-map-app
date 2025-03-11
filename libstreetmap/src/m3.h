#include "ezgl/graphics.hpp"
#include "ezgl/canvas.hpp"
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include <vector>
using namespace std;

struct Node
{
    StreetSegmentIdx from;
    StreetSegmentIdx to;
}


void drawPath(ezgl::renderer *g, vector<StreetSegmentIdx> path); //Draws collection of street segments on map as path