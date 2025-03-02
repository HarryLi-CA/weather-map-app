#include "renderHelper.h"
#include <vector>
#include "StreetsDatabaseAPI.h"
#include "m1.h"
#include <stdio.h>
#include <map>

using namespace std;

static double averageLongitude = 0;
static double averageLatitude = 0;

void setLatLonBounds(LatLon latlonMin, LatLon latlonMax)
{
    averageLatitude = (latlonMin.latitude() + latlonMax.latitude())/2.0;
    averageLongitude = (latlonMin.longitude() + latlonMax.longitude())/2.0;
}


MapFeature::MapFeature(int _layer, vector<ezgl::point2d> *_points, ezgl::color _color, DrawType _drawType, MapFeatureType _featureType, int _index)
{
    //apply arguments to corrosponding variables
    layer = _layer;

    points = _points;
    bounds = boundingRectangle(points);
    
    drawType = _drawType;
    featureType = _featureType;

    dataIndex = _index;

    color = _color;

    //set default values
    wasRendered = false;
    highlight = false;
    streetID = -1;
    major = false;
    inCellList = false;
}

FeatureList::FeatureList(){
    //set default values
    highlightedFeature = nullptr;
    highlightedStreetID = -2;
}

void FeatureList::highlightFeature(MapFeature* feature)
{
    highlightedStreetID = -2; //reset highlighted street ID
    //if a feature is currently highlighted, remove the highlight (only one feature should be highlighted at a time)
    if(highlightedFeature != nullptr)
    {
        highlightedFeature->highlight = false;
    }
    //highlight new feature
    feature->highlight = true;
    //if feature is a street segment, highlight entire street
    if(feature->streetID > 0) //0 is a default street id, so avoid setting the highlighted street id to 0
    {
        highlightedStreetID = feature->streetID;
    }
    //store feature so it can be unhighlighted if needed
    highlightedFeature = feature;
}

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

ezgl::rectangle boundingRectangle(const vector<ezgl::point2d>* points)
{
    double minY = DBL_MAX; //minimum y value in point list
    double minX = DBL_MAX; //minimum x value in point list
    double maxY = -DBL_MAX; //maximum y value in point list
    double maxX = -DBL_MAX; //maximum x value in point list

    for(int pointIndex = 0; pointIndex < points->size(); pointIndex++)
    {
        ezgl::point2d point = (*points)[pointIndex];
        if(point.x > maxX){maxX = point.x;} //check for maximum X
        if(point.y > maxY){maxY = point.y;} //check for maximum Y
        if(point.x < minX){minX = point.x;} //check for minimum X
        if(point.y < minY){minY = point.y;} //check for minimum Y
    }

    ezgl::rectangle boundingRect({minX, minY},{maxX,maxY}); //create bounding rectangle
    return boundingRect; //return created rectangle as bounding rectangle
}

bool intersecting(ezgl::rectangle rectA, ezgl::rectangle rectB)
{
    bool outside = rectA.left() > rectB.right() || rectA.right() < rectB.left() || rectA.top() < rectB.bottom() || rectA.bottom() > rectB.top();
    return !outside;
}

pair<int,int> getRenderCell(double originX, double originY, double cellSize, ezgl::point2d position)
{
    return {(position.x - originX)/cellSize, (position.y - originY)/cellSize};
} 

vector<pair<int,int>> getCurrentRenderCells(ezgl::camera* camera, double cellSize, LatLon bottomLeft)
{
    return getSafeBoundedRenderCells(camera->get_world(), cellSize, bottomLeft);
}

vector<pair<int,int>> getSafeBoundedRenderCells(ezgl::rectangle rect, double cellSize, LatLon bottomLeft)
{
    vector<pair<int,int>> cells;
    //get relative origin x and y
    double originY = LatToY(bottomLeft.latitude());
    double originX = LonToX(bottomLeft.longitude());

    for(double y = rect.bottom() - cellSize; y < rect.top() + cellSize; y += cellSize)
    {
        for(double x = rect.left() - cellSize; x < rect.right() + cellSize; x += cellSize)
        {
            pair<int, int> cell = getRenderCell(originX, originY, cellSize, {x, y}); //calculate cell
            cells.push_back(cell); //add cell to cells list
        }
    }
    return cells;
}

vector<pair<int,int>> getBoundedRenderCells(ezgl::rectangle rect, double cellSize, LatLon bottomLeft)
{
    vector<pair<int,int>> cells;
    //get relative origin x and y
    double originY = LatToY(bottomLeft.latitude());
    double originX = LonToX(bottomLeft.longitude());

    for(double y = rect.bottom(); y < rect.top(); y += cellSize)
    {
        for(double x = rect.left(); x < rect.right(); x += cellSize)
        {
            pair<int, int> cell = getRenderCell(originX, originY, cellSize, {x, y}); //calculate cell
            cells.push_back(cell); //add cell to cells list
        }
    }
    return cells;
}

void addToRenderCellMap(pair<int, int> cell, MapFeature* feature, map<pair<int, int>, vector<MapFeature*>*>* cellMap)
{
    //check if cell already in cell map
    if(cellMap->find(cell) == cellMap->end())
    {
        //if not, create vector at that key
        vector<MapFeature*>* mapFeatureList = new vector<MapFeature*>; //list of map features to draw
        cellMap->insert({cell, mapFeatureList});
    }
    (*cellMap)[cell]->push_back(feature);
}

vector<MapFeature*> getFeaturesFromCells(vector<pair<int,int>> cells, map<pair<int, int>, vector<MapFeature*>*>&cellMap)
{
    vector<MapFeature*> features;
    for(auto cell = cells.begin(); cell < cells.end(); cell++)
    {
        vector<MapFeature*>* featureList = (cellMap[*cell]);
        if(featureList == nullptr){continue;}
        for(auto feature = featureList->begin(); feature != featureList->end(); feature++)
        {
            if((*feature)->inCellList){continue;}
            (*feature)->inCellList = true;
            features.push_back(*feature);
        }
    }
    for(auto feature = features.begin(); feature != features.end(); feature++)
    {
        (*feature)->inCellList = false;
    }
    return features;
}