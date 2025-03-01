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
    MapFeature(int _layer, vector<ezgl::point2d> *_points, DrawType _drawType);
};

//customized container for storing and sorting map features
class FeatureList {
    public:
    map<int, vector<MapFeature*>*> layerMap; //map to store layers
    FeatureList();
    void addFeature(MapFeature* feature);
};

ezgl::point2d LatLonToPoint2D(LatLon maxPoints, LatLon minPoints, LatLon latlonPoint, double scale); //converts a latitude/longitude point to an ezgl point2d object