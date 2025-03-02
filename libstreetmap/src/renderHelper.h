#pragma once
#include "ezgl/canvas.hpp";
#include "StreetsDatabaseAPI.h"
#include <map>

using namespace std;

//speicifes how renderer will draw features
enum DrawType {
    POINT,
    LINE,
    POLYGON
};

//container for an object on a map that specifies how it will be drawn relative to other features
class MapFeature {
    public:
    int layer; //layer to draw the feature. low layers will be drawn first.
    vector<ezgl::point2d>* points; //position of vertices of feature relative to absolute origin.
    DrawType drawType; //method renderer will use to draw feature.
    double maximumScreenLength; //maximum screen length until this feature starts to become invisible
    ezgl::color color;
    MapFeature(int _layer, vector<ezgl::point2d> *_points, DrawType _drawType, double _maximumScreenLength, ezgl::color _color);
};

//customized container for storing and sorting map features
class FeatureList {
    public:
    map<int, vector<MapFeature*>*> layerMap; //map to store layers
    FeatureList();
    void addFeature(MapFeature* feature);
};

void setLatLonBounds(LatLon latlonMin, LatLon latlonMax); //setup constants for latlon to cartesian conversions using minimum and maximum latlon bounds
double LatToY(double lat); //converts latitude angle to cartesian y coordinate
double LonToX(double lon); //converts longitude angle to cartesian x coordinate 
ezgl::point2d LatLonToPoint2D(LatLon latlonPoint); //converts a latitude/longitude point to an ezgl point2d object
void updateMaximumLatLon(double& minimumLatitude, double& minimumLongitude, double& maximumLatitude,double& maximumLongitude, LatLon point); //updates minimum and maximum latitude/longitude values based on an input point
int lengthToLayer(double length); //helper function to determine the layer of a length of road based on its length 
double lengthToLayerInterpolate(double length); //helper function to determine the ratio between layers of a length of road based on its length