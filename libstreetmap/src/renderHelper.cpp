#include "renderHelper.h"


MapFeature::MapFeature(int _layer, vector<ezgl::point2d> *_points, DrawType _drawType)
{
    layer = _layer;
    points = _points;
    drawDimension = _drawType;
}