#include "renderHelper.h"
#include "ezgl/point.hpp"
#include "ezgl/rectangle.hpp"
#include <vector>
#include "StreetsDatabaseAPI.h"
#include "m1.h"
#include <stdio.h>
#include <map>
#include "QuadTree.h"
#include "pointMath.h"
#include "loading_utils.h"
#include <stdlib.h>

using namespace std;

static double averageLongitude = 0;
static double averageLatitude = 0;
const double kBoundSizeStep = 100000; //discrete steps for bound area
const double kGraphicsPixelToMeters = 1.0;

void setLatLonBounds(LatLon latlonMin, LatLon latlonMax)
{
    averageLatitude = (latlonMin.latitude() + latlonMax.latitude())/2.0;
    averageLongitude = (latlonMin.longitude() + latlonMax.longitude())/2.0;
}


Graphic::Graphic(const char* _path)
{
    filePath = _path;
}

void Graphic::load(ezgl::renderer* g)
{
    surface = g->load_png(filePath);
    width = cairo_image_surface_get_width(surface);
    height = cairo_image_surface_get_width(surface);
}

void Graphic::draw(ezgl::renderer* g, double worldScale, ezgl::point2d position, ezgl::point2d offset, double imageScale, ezgl::color color)
{
    cairo_set_operator((cairo_t*)surface, cairo_operator_t::CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba((cairo_t*)surface, color.red, color.green, color.blue, color.alpha);
    draw(g, worldScale, position, offset, imageScale);
}

void Graphic::draw(ezgl::renderer* g, double worldScale, ezgl::point2d position, ezgl::point2d offset, double imageScale)
{
    double drawScale = kGraphicsPixelToMeters/worldScale;
    g->draw_surface(surface,{position.x + offset.x*drawScale*height*imageScale, position.y + offset.y*drawScale*height*imageScale},imageScale);
}

void Graphic::deallocate(ezgl::renderer* g)
{
    g->free_surface(surface);   
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
    directional = false;
    maximumScaling = DBL_MAX;
    longestPointSegmentIndex = -1;
    longestSegmentLength = 0;

    //calculate area
    thresholdArea = calculateArea();
}

MapFeature::~MapFeature()
{
    //delete points
    delete  points;
    points = nullptr;
}

double MapFeature::calculateArea()
{
    double worldArea = bounds.area();
    if(drawType == DrawType::POLYGON)
    {
        worldArea = findFeatureArea(dataIndex);
    }
    return worldArea;
}

FeatureList::FeatureList(){
    //set default values
    highlightedFeatures = vector<MapFeature*>();
    highlightedStreetID = -2;
    
}

void FeatureList::clear()
{
    //delete all features in each layer
    auto layer = layerMap.begin();
    while(layer != layerMap.end())
    {
        vector<MapFeature *> * curVector = (layer->second);
        deleteVectorPointers(*curVector);

        (*layer->second).clear();

        delete curVector;
        (layer->second) = nullptr;

        layer++;
    }
    layerMap.clear();
    spatialIndexer.clear();
    cout << "done clearing spatial indexer" << endl;
    highlightedFeatures = {};
    numberOfFeatures = 0;
}

bool getDrawPriority(MapFeature* a, MapFeature* b) //helper function to determine what features should be drawn first
{
    bool basicPriority = a->layer < b->layer; //higher layers should be drawn last
    if(a->layer == b->layer)
    {
        if(a->drawType == DrawType::POLYGON && b->drawType == DrawType::POLYGON)
        {
            basicPriority = a->thresholdArea > b->thresholdArea; //large polygons should be drawn first
        }
        else
        {
            basicPriority = a->dataIndex < b->dataIndex; //higher data indices should be drawn last on the same layer
        }
    }
    return basicPriority;
}

void FeatureList::highlightFeature(MapFeature* feature, bool unhighlightOthers)
{
    highlightedStreetID = -2; //reset highlighted street ID
    //if a feature is currently highlighted, remove the highlight (only one feature should be highlighted at a time)
    if(unhighlightOthers){
        for(int featureIndex = 0; featureIndex < highlightedFeatures.size(); featureIndex++)
        {
            highlightedFeatures[featureIndex]->highlight = false;
        }
        highlightedFeatures = {};
    }
    //highlight new feature
    feature->highlight = true;
    //if feature is a street segment, highlight entire street
    if(feature->streetID > 0) //0 is a default street id, so avoid setting the highlighted street id to 0
    {
        highlightedStreetID = feature->streetID;
    }
    //store feature so it can be unhighlighted if needed
    highlightedFeatures.push_back(feature);
}

void FeatureList::unhighlightAll()
{
    for(int featureIndex = 0; featureIndex < highlightedFeatures.size(); featureIndex++)
    {
        highlightedFeatures[featureIndex]->highlight = false;
    }
    highlightedFeatures = {};
}

void FeatureList::addFeature(MapFeature* feature, double areaThreshold, double worldArea)
{
    bool lowLOD = feature->major || (feature->drawType == DrawType::POLYGON && feature->thresholdArea > areaThreshold) || feature->drawType == DrawType::POINT; //should feature be rendered at high distances
    
    //add feature to list of features
    if(layerMap.count(feature->layer) == 0) //check if layer exists
    {
        //if not, create layer and add it to map
        vector<MapFeature*>* featureLayer = new vector<MapFeature*>; 
        layerMap.insert({feature->layer,featureLayer});
    }
    layerMap[feature->layer]->push_back(feature); //add feature to corrosponding layer

    //low LOD features should be added to manual layer system
    
    if(lowLOD)
    {
        spatialIndexer.insertBounds({feature->bounds.center(),feature->bounds,worldArea/4,feature,feature->layer}); //add feature to top layer
        return;
    }
    double boundsSize = ((int)(feature->bounds.area()/kBoundSizeStep))*kBoundSizeStep;
    boundsSize = max(boundsSize, (double)kBoundSizeStep); //make sure area is non-zero
    //add every point to spatial indexing
    if(feature->drawType != DrawType::POINT) //large objects and lines should be added in multiple times to prevent clipping
    {
    if(feature->dataIndex == 2054866){
        cout << "test6"  << feature->layer<<endl;
    }
        spatialIndexer.insertBounds({feature->bounds.center(),feature->bounds,boundsSize,feature,feature->layer});
    if(feature->dataIndex == 2054866){
        cout << "test7"  << feature->layer<<endl;
    }
    }
    else
    {
        spatialIndexer.insert({feature->bounds.center(),feature->bounds,boundsSize,feature,feature->layer});
    }
    

    
    numberOfFeatures++;
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

void mapRenderCells(double renderCellSize, LatLon bottomLeft, FeatureList& mapFeatures, map<pair<int, int>, vector<MapFeature*>*>& cellMap)
{
   auto layer = mapFeatures.layerMap.begin();
   double originY = LatToY(bottomLeft.latitude());
   double originX = LonToX(bottomLeft.longitude());

   while(layer != mapFeatures.layerMap.end())
   {
      for(int i = 0; i < layer->second->size(); i++)
      {
         
         MapFeature* feature = (*(layer->second))[i];
         vector<pair<int, int>> cells;
         switch(feature->drawType)
         {
            case POINT: {
               //only print points if they are highlighted
               cells.push_back(getRenderCell(originX,originY,renderCellSize,(*feature->points)[0]));
               break;
            }
            case LINE: {
               //calculate the transparency of a street based on its length
               ezgl::rectangle& bounds = feature->bounds;
               cells = getBoundedRenderCells(bounds,renderCellSize,bottomLeft);
               break;
            }
            case POLYGON: {
               ezgl::rectangle& bounds = feature->bounds;
               cells = getBoundedRenderCells(bounds,renderCellSize,bottomLeft);
               break;
            }
            default: {
                break;
            }
         }

         for(auto cell = cells.begin(); cell != cells.end(); cell++)
         {
            addToRenderCellMap(*cell,feature,&cellMap);
         }
         
      }
      layer++;
   }
}

vector<MapFeature*> getFeaturesFromCells(vector<pair<int,int>> cells, map<pair<int, int>, vector<MapFeature*>*>&cellMap)
{
    vector<MapFeature*> features; //vector of features to display
    for(auto cell = cells.begin(); cell < cells.end(); cell++)
    {
        vector<MapFeature*>* featureList = (cellMap[*cell]); //features partially located in this cell
        if(featureList == nullptr){continue;} //if no featurs are in this cell (indicated by nullptr), skip cell
        for(auto feature = featureList->begin(); feature != featureList->end(); feature++) //iterate through all features in list
        {
            if((*feature)->inCellList){continue;} //if feature is already in vector to be displayed, do not duplicate it
            (*feature)->inCellList = true; //flag feature as already in display list
            features.push_back(*feature); //add feature to display list
        }
    }
    //reset "in display list" flag for all added features
    for(auto feature = features.begin(); feature != features.end(); feature++)
    {
        (*feature)->inCellList = false;
    }
    return features; //return vector of features to display
}

void drawTriangle(ezgl::renderer *g, ezgl::point2d center, ezgl::point2d direction, ezgl::color color, double distanceFromCenter)
{
   double angle = atan2(direction.y, direction.x);
   //draw triangle based on initial angle
   g->set_color(color);
   g->fill_poly({
      {center.x + 1.5*distanceFromCenter*cos(angle), center.y + 1.5*distanceFromCenter*sin(angle)},
      {center.x + distanceFromCenter*cos(angle+2*M_PI/3.0), center.y + distanceFromCenter*sin(angle+2*M_PI/3.0)},
      {center.x + distanceFromCenter*cos(angle+4*M_PI/3.0), center.y + distanceFromCenter*sin(angle+4*M_PI/3.0)},
   });
}



string getKnownIntersectionName(IntersectionIdx index)
{
   string intersection = getIntersectionName(index);
   //remove <unknown>s from features
   int unknownIndex = intersection.find("<unknown> &");
   if(unknownIndex > -1)
   {  
      intersection = intersection.replace(unknownIndex, 11,"");
   }

   unknownIndex = intersection.find("<unknown>");
   if(unknownIndex > -1)
   {  
      intersection = intersection.replace(unknownIndex, 9,"");
   }
   return intersection;
}

ezgl::point2d getMiddleOfStreetSegment(StreetSegmentIdx segmentIndex)
{
    vector<LatLon> points = *getStreetSegmentPoints(segmentIndex); //curve points of street segment
    double totalLength = findStreetSegmentLength(segmentIndex); //total length of street segment
    double currentLength = 0; //running total length of street segment
    for(int curveIndex = 0; curveIndex < points.size() - 1; curveIndex++)
    {
        //get points
        LatLon curvePoint = points[curveIndex]; //current curve point
        LatLon nextPoint = points[curveIndex + 1]; //next curve point
        //add subsegment length to total segment length
        double subsegmentLength = findDistanceBetweenTwoPoints(curvePoint, nextPoint);
        //if this segment is the first to increase the current length to over half the total length, this segment is in the middle
        cout << currentLength  << "+" << subsegmentLength << " / " << totalLength/2.0 << endl;
        if(currentLength + subsegmentLength >= totalLength/2.0)
        {
            ezgl::point2d fromPoint = LatLonToPoint2D(curvePoint); //first point in the segment
            ezgl::point2d toPoint = LatLonToPoint2D(nextPoint); //first point in the segment

            ezgl::point2d directionVector = normalize(fromPoint - toPoint); //get direction of subsegment

            double requiredDistance = totalLength/2.0 - currentLength; //get distance from

            ezgl::point2d relativeMiddle = {directionVector.x*requiredDistance, directionVector.y*requiredDistance}; //get midpoint relative to from ponit
            ezgl::point2d middlePoint = fromPoint + relativeMiddle; //get midpoint relative to world

            return middlePoint; //return middlepoint

        }
        //add subsegment length to running total
        currentLength += subsegmentLength;
    }
    //if no point is found, return zero
    return {0.0, 0.0};
}

ezgl::color staticRGBScale(ezgl::color rgb, double scale)
{
    //safely cast scaled rgb values
    u_int8_t r = static_cast<u_int8_t>(rgb.red*scale); //scaled red
    u_int8_t g = static_cast<u_int8_t>(rgb.green*scale); //scaled green
    u_int8_t b = static_cast<u_int8_t>(rgb.blue*scale); //scaled blue

    return {r,g,b,rgb.alpha}; //return scaled values as color object
}