#pragma once
#include "ezgl/canvas.hpp"
#include "StreetsDatabaseAPI.h"
#include <map>
#include "ezgl/point.hpp"
#include "ezgl/rectangle.hpp"
#include "QuadTree.h"

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
    double maximumScaling; //maximum meters per pixel for the feature to start drawing
    double thresholdArea; //calculated bounding area of the feature at the cell rendering threshold screen-to-world ratio
    double virtualBoundsArea;
    StreetIdx streetID; //corrosponding streetID of a map feature (only applicable for street segments, -1 by default)
    bool wasRendered; //cached value to store whether the feature was rendered this frame
    bool wasOutlined; //cached value to store where the feature's outline was rendered this frame
    ezgl::color color; //color to draw the feature in
    bool highlight; //if true, the feature will be outlined (polygon) or highlighted (line)
    bool major; //significant feature that should be displayed at larger distances (i.e major road, POIs)
    bool inCellList; //was added to cell list for rendering this frame
    bool directional; //is the road strictly unidirectional (only applicable to roads)
    int longestPointSegmentIndex; //index before the longest segment of a curved line (only applicable to roads)
    double longestSegmentLength; //length of the longest segment of a turn segment (only applicable to roads)
    double calculateArea(); //returns bounding area of map feature
    MapFeature(int _layer, vector<ezgl::point2d> *_points, ezgl::color _color, DrawType _drawType, MapFeatureType _featureType, int _index);
    ~MapFeature();
};

//customized container for storing and sorting map features
class FeatureList {
    public:
    map<int, vector<MapFeature*>*> layerMap; //map to store layers
    QuadTree<MapFeature> spatialIndexer;
    vector<MapFeature*> highlightedFeatures; //currently highlighted feature
    StreetIdx highlightedStreetID; //currently highlighted street (-2 by default)
    int numberOfFeatures = 0;
    int numerOfLowLODFeatures = 0;
    FeatureList();
    void addFeature(MapFeature* feature, double areaThreshold, double worldArea); //adds feature to the map at the corresponding layer
    void highlightFeature(MapFeature* feature, bool unhighlightOthers); //highlights feature and unhighlights currently highlighted features
    void clear(); //deletes all vectors and map features in the layer map
};

//structure used to define text to draw
typedef struct staticname_t {
    string text; //test to draw
    ezgl::point2d position; //world position to draw text at
    MapFeature* feature; //map feature this text object is connected to

}StaticName;

bool getDrawPriority(MapFeature* a, MapFeature* b); //helper function to determine which map feature should be rendered first

void setLatLonBounds(LatLon latlonMin, LatLon latlonMax); //setup constants for latlon to cartesian conversions using minimum and maximum latlon bounds

double LatToY(double lat); //converts latitude angle to cartesian y coordinate
double LonToX(double lon); //converts longitude angle to cartesian x coordinate 

ezgl::point2d LatLonToPoint2D(LatLon latlonPoint); //converts a latitude/longitude point to an ezgl point2d object

void updateMaximumLatLon(double& minimumLatitude, double& minimumLongitude, double& maximumLatitude,double& maximumLongitude, LatLon point); //updates minimum and maximum latitude/longitude values based on an input point

int lengthToLayer(double length); //helper function to determine the layer of a length of road based on its length 
double lengthToLayerInterpolate(double length); //helper function to determine the ratio between layers of a length of road based on its length

bool insideRectangle(ezgl::rectangle rect, vector<ezgl::point2d>* points); //helper function to determine if a shape formed by a list of points are all outside rect
ezgl::rectangle boundingRectangle(const vector<ezgl::point2d>* points); //helper function to create rectangle that perfectly contains set of points

vector<pair<int,int>> getCurrentRenderCells(ezgl::camera* camera, double cellSize, LatLon bottomLeft); //helper function to get current render cell based on cellSize and camera position
pair<int,int> getRenderCell(double originX, double originY, double cellSize, ezgl::point2d position); //returns render cell at a position
vector<pair<int,int>> getBoundedRenderCells(ezgl::rectangle rect, double cellSize, LatLon bottomLeft); //returns all render cells inside of a given rectangle
vector<pair<int,int>> getSafeBoundedRenderCells(ezgl::rectangle rect, double cellSize, LatLon bottomLeft); //returns all render cells inside of a given rectangle, adding a 1 cell buffer around the rectangle

void addToRenderCellMap(pair<int, int> cell, MapFeature* feature, map<pair<int, int>, vector<MapFeature*>*>* cellMap); //inserts cells into cell map and assigns new vectors when needed
vector<MapFeature*> getFeaturesFromCells(vector<pair<int,int>> cells, map<pair<int, int>, vector<MapFeature*>*>& cellMap); //returns all features corrosponding to a set of render cells
void mapRenderCells(double renderCellSize, LatLon bottomLeft, FeatureList& mapFeatures, map<pair<int, int>, vector<MapFeature*>*>& cellMap); //maps all features ina feature list based on their render cells to a render cell map

void drawTriangle(ezgl::renderer *g, ezgl::point2d center, ezgl::point2d direction, ezgl::color color, double distanceFromCenter); //draws triangle around a center point

ezgl::point2d getMiddleOfStreetSegment(StreetSegmentIdx segmentIndex); //helper function that returns the point that lies in the middle of a street segment (curved or non-curved)


string getKnownIntersectionName(IntersectionIdx index); //helper function to get an intersection name while removing "<unknown> &" from the name

template <typename T> bool vectorContains(vector<T>& vec, T& toFind) //returns true if toFind is an element of vec
{
    //iterate through vector
    for(int index = 0; index < vec.size(); index++)
    {
        //if vector has object, return true
        if(vec[index] == toFind)
        {
            return true;
        }
    }
    //if entire vector is search and no object is found, return false
    return false;
}