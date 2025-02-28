#include "renderHelper.h"
#include <vector>

using namespace std;

MapFeature::MapFeature(int _layer, vector<ezgl::point2d> *_points, DrawType _drawType)
{
    layer = _layer;
    points = _points;
    drawType = _drawType;
}