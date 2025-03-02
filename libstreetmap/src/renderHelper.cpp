#include "renderHelper.h"
#include <vector>
#include "StreetsDatabaseAPI.h"
#include "m1.h"
#include <stdio.h>

using namespace std;

static double averageLongitude = 0;
static double averageLatitude = 0;

void setLatLonBounds(LatLon latlonMin, LatLon latlonMax)
{
    averageLatitude = (latlonMin.latitude() + latlonMax.latitude())/2.0;
    averageLongitude = (latlonMin.longitude() + latlonMax.longitude())/2.0;
}


MapFeature::MapFeature(int _layer, vector<ezgl::point2d> *_points, DrawType _drawType, double _maximumScreenLength, ezgl::color _color)
{
    layer = _layer;
    points = _points;
    drawType = _drawType;
    maximumScreenLength = _maximumScreenLength;
    color = _color;
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


double LatToY(double lat)
{
    return kEarthRadiusInMeters * lat *kDegreeToRadian;
}


double LonToX(double lon)
{
    return (lon * kDegreeToRadian * kEarthRadiusInMeters * cos(averageLatitude * kDegreeToRadian));
}

ezgl::point2d LatLonToPoint2D(LatLon latlonPoint)
{
    double cartX = LonToX(latlonPoint.longitude()); //get cartesian x value from longitude
    double cartY = LatToY(latlonPoint.latitude()); //get cartesian y value from latitude
    return {cartX, cartY}; //create and return ezgl point2d object containing cartesian coordinates
}

void updateMaximumLatLon(double& minimumLatitude, double& minimumLongitude, double& maximumLatitude,double& maximumLongitude, LatLon point)
{
   //check if latitude is higher than recorded maximum latitude
   if(point.latitude() > maximumLatitude)
   {
      maximumLatitude = point.latitude(); //replace recorded maximum with new value
   }
   //check if longitude is higher than recorded maximum latitude
   if(point.longitude() > maximumLongitude)
   {
      maximumLongitude = point.longitude(); //replace recorded maximum with new value
   }
   //check if latitude is higher than recorded maximum latitude
   if(point.latitude() < minimumLatitude)
   {
      minimumLatitude = point.latitude(); //replace recorded minimum with new value
   }
   //check if longitude is higher than recorded maximum latitude
   if(point.longitude() < minimumLongitude)
   {
      minimumLongitude = point.longitude(); //replace recorded minimum with new value
   }
}

int lengthToLayer(double length)
{
   return (int)lengthToLayerInterpolate(length);
}

double lengthToLayerInterpolate(double length)
{
   return length/100.0;
}