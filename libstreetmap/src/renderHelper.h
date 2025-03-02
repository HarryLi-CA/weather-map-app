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

enum MapFeatureType {
    INTERSECTION,
    ROAD,
    FEATURE
};

//container for an object on a map that specifies how it will be drawn relative to other features
class MapFeature {
    public:
    int layer; //layer to draw the feature. low layers will be drawn first.
    vector<ezgl::point2d>* points; //position of vertices of feature relative to absolute origin.
    ezgl::rectangle bounds; //bounding rectangle for points
    DrawType drawType; //method renderer will use to draw feature.
    MapFeatureType featureType; //type of feature
    int dataIndex; //index to recover information (either street segment or feature)
    StreetIdx streetID; //corrosponding streetID of a map feature (only applicable for street segments, -1 by default)
    bool wasRendered; //cached value to store whether the feature was rendered this frame
    ezgl::color color; //color to draw the feature in
    bool highlight; //if true, the feature will be outlined (polygon) or highlighted (line)
    MapFeature(int _layer, vector<ezgl::point2d> *_points, ezgl::color _color, DrawType _drawType, MapFeatureType _featureType, int _index);
};

//customized container for storing and sorting map features
class FeatureList {
    public:
    map<int, vector<MapFeature*>*> layerMap; //map to store layers
    MapFeature* highlightedFeature; //currently highlighted feature
    StreetIdx highlightedStreetID; //currently highlighted street (-2 by default)
    FeatureList();
    void addFeature(MapFeature* feature);
    void highlightFeature(MapFeature* feature);
};

//structure used to define text to draw
typedef struct staticname_t {
    string text; //test to draw
    ezgl::point2d position; //world position to draw text at
    MapFeature* feature; //map feature this text object is connected to

}StaticName;

void setLatLonBounds(LatLon latlonMin, LatLon latlonMax); //setup constants for latlon to cartesian conversions using minimum and maximum latlon bounds
double LatToY(double lat); //converts latitude angle to cartesian y coordinate
double LonToX(double lon); //converts longitude angle to cartesian x coordinate 
ezgl::point2d LatLonToPoint2D(LatLon latlonPoint); //converts a latitude/longitude point to an ezgl point2d object
void updateMaximumLatLon(double& minimumLatitude, double& minimumLongitude, double& maximumLatitude,double& maximumLongitude, LatLon point); //updates minimum and maximum latitude/longitude values based on an input point
int lengthToLayer(double length); //helper function to determine the layer of a length of road based on its length 
double lengthToLayerInterpolate(double length); //helper function to determine the ratio between layers of a length of road based on its length
bool insideRectangle(ezgl::rectangle rect, vector<ezgl::point2d>* points); //helper function to determine if a shape formed by a list of points are all outside rect
ezgl::rectangle boundingRectangle(const vector<ezgl::point2d>* points); //helper function to create rectangle that perfectly contains set of points
bool intersecting(ezgl::rectangle rectA, ezgl::rectangle rectB); //helper functoin to check if 2 rectangles are intersecting