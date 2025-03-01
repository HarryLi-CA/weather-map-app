#include "renderHelper.h"
#include <vector>
#include "StreetsDatabaseAPI.h"
#include "m1.h"
#include <stdio.h>

using namespace std;

MapFeature::MapFeature(int _layer, vector<ezgl::point2d> *_points, DrawType _drawType)
{
    layer = _layer;
    points = _points;
    drawType = _drawType;
}

FeatureList::FeatureList(){}

void FeatureList::addFeature(MapFeature* feature)
{
    //check if layer exists
    if(layerMap.count(feature->layer) == 0)
    {
        //if not, create layer and add it to map
        vector<MapFeature*>* featureLayer = new vector<MapFeature*>; 
        layerMap.insert({feature->layer,featureLayer});
    }
    layerMap[feature->layer]->push_back(feature); //add feature to corrosponding layer
}

ezgl::point2d LatLonToPoint2D(LatLon maxPoints, LatLon minPoints, LatLon latlonPoint, double scale)
{
    LatLon commonHorizontalLine = LatLon(latlonPoint.latitude(), minPoints.longitude()); //latlon point that shares a latitude with the target point and longitude with the minimum point
    LatLon commonVerticalLine = LatLon(minPoints.latitude(), latlonPoint.longitude()); //latlon point that shares a longitude with the target point and latitude with the minimum point 
    
    LatLon totalHorizontalLineMax = LatLon(minPoints.latitude(), maxPoints.longitude());
    LatLon totalVerticalLineMax = LatLon(maxPoints.latitude(), maxPoints.longitude());
    
    double totalWidth = findDistanceBetweenTwoPoints(minPoints, totalHorizontalLineMax);
    double totalHeight = findDistanceBetweenTwoPoints(minPoints, totalVerticalLineMax);
    //cout << latlonPoint.longitude() << ", " << maxPoints.longitude() << endl;
    double xDifference = scale*findDistanceBetweenTwoPoints(commonHorizontalLine, latlonPoint)/totalWidth; //horizontal distance from vertical line aligned with minimum longitude
    double yDifference = scale*findDistanceBetweenTwoPoints(commonVerticalLine, latlonPoint)/totalHeight; //horizontal distance from vertical line aligned with minimum longitude
    return ezgl::point2d(xDifference, yDifference); //return point with distances as coordinate points
}