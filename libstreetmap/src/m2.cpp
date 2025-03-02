/* 
 * Copyright 2025 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated 
 * documentation files (the "Software") in course work at the University 
 * of Toronto, or for personal use. Other uses are prohibited, in 
 * particular the distribution of the Software either publicly or to third 
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "m2.h"
#include "m1.h"
#include "renderHelper.h"
#include <vector>
#include <map>
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "nodeIDHelper.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "ezgl/camera.hpp"

using namespace std;

static ezgl::rectangle initial_world{{0, 0}, 1920, 1080}; //rendered canvas
static FeatureList mapFeatures; //list of map features to draw
static ezgl::canvas* mainCanvas; //application canvas
LatLon minPoints; //bottom left LatLon position
LatLon maxPoints; //top right LatLon position
vector<StaticName> staticLabels; //list of static text objects to draw
vector<StreetIdx> streetLengthCache; //stored value of all loaded street lengths;
map<pair<int, int>, vector<MapFeature*>*> cellMap; //map of all render cells to list of features
//default colour constants for general objects
const ezgl::color kColorDefaultGround = {235, 235, 235, 255}; 
const ezgl::color kColorHighlight = {255, 87, 87, 255};
//default colour constants for features
const ezgl::color kColorScrub = {212, 247, 178, 255};
const ezgl::color kColorWood = {138, 191, 134, 255};
const ezgl::color kColorBuilding = {166, 166, 166, 255};
const ezgl::color kColorWater = {147, 194, 230, 255};
const ezgl::color kColorBeach = {242,242,184, 255};
const ezgl::color kColorIsland = {238, 242, 213, 255};
//default colour constants for roads
const ezgl::color kColorBasicRoad = {140, 140, 140, 255};
const ezgl::color kColorOnewayRoad = {219, 118, 118, 255};
const ezgl::color kColorMajorRoad = {0, 0, 200, 255};
const ezgl::color kColorOnewayMajorRoad = {148,0,211, 255};
//define rendering constants
const int kMinimumPixelsToDraw = 10; //minimum pixels a feature should take up on the screen to be drawn that cycle
const double kMinimumMajorRoadLength = 5000; //minimum length for a road to be considered a major road
const double kRenderCellSize = 300; //size of each render cell
const int kMaximumRenderCells = 100; //maximum number of render cells before switching to direct rendering


void drawFeature(MapFeature *feature, ezgl::renderer *g, double screenArea, double lineWidth, double scaling, ezgl::camera camera)
{
   switch(feature->drawType)
      {
         case POINT: {
            //only print points if they are highlighted
            feature->wasRendered = false;
            if(!feature->highlight){break;}
            ezgl::point2d origin = (*(feature->points))[0];
            g->set_color(feature->color);
            g->draw_arc(origin,10.0,0.0,360.0);
            
            feature->wasRendered = true; //feature was fully rendered this cycle
            break;
         }
         case LINE: {
            feature->wasRendered = false;
            bool highlightedStreet = mapFeatures.highlightedStreetID == feature->streetID; //check if segment is on highlighted street
            //calculate the transparency of a street based on its length
            ezgl::rectangle& bounds = feature->bounds;
            ezgl::rectangle screen_rect(camera.world_to_screen(bounds.bottom_left()),camera.world_to_screen(bounds.top_right()));
            if(!feature->major && !highlightedStreet && screen_rect.area() < kMinimumPixelsToDraw){break;} 
            
            g->set_color(feature->color);
            if(highlightedStreet)
            {
               g->set_color(kColorHighlight);
            }
            //if major road, make road wider as indication 
            if(feature->major)
            {
               g->set_line_width(lineWidth*2);
            }
            for(int i = 0; i < feature->points->size() - 1; i++)
            {
               g->draw_line((*feature->points)[i],(*feature->points)[i+1]);
            }
            
            
            feature->wasRendered = true; //feature was fully rendered this cycle
            break;
         }
         case POLYGON: {
            feature->wasRendered = false;
            //dont render small features
            const ezgl::rectangle& bounds = feature->bounds;
            
            const ezgl::rectangle screen_rect(camera.world_to_screen(bounds.bottom_left()),camera.world_to_screen(bounds.top_right()));
            if(!feature->highlight && screen_rect.area() < 10){break;} 
         
            g->set_color(feature->color);
            g->fill_poly(*(feature->points));

            if(feature->highlight)
            {
               g->set_color(kColorHighlight);
               g->set_line_width(1);
               for(int i = 0; i < feature->points->size() - 1; i++)
               {
                  g->draw_line((*feature->points)[i],(*feature->points)[i+1]);
               }
            }

            feature->wasRendered = true; //feature was fully rendered this cycle
            break;
         }
      }
}

void draw_main_canvas(ezgl::renderer *g)
{
   ezgl::rectangle rect(initial_world.bottom_left(),initial_world.top_right());
   
   g->set_color(kColorDefaultGround);
   g->fill_rectangle(rect);
   auto layer = mapFeatures.layerMap.begin();
   
   double zoom = getScaleFactor();
   double absoluteLength = findDistanceBetweenTwoPoints(minPoints, maxPoints);

   double pixelLength = mainCanvas->get_camera().get_screen().width();
   double pixelHeight = mainCanvas->get_camera().get_screen().height();
   double screenLength = mainCanvas->get_camera().get_world().width();
   double screenHeight = mainCanvas->get_camera().get_world().height();
   double scaling = pixelLength/screenLength;
   double screenArea = screenLength*screenHeight;
   double invisibilityFactor = 0.5/absoluteLength;
   int lineWidth = max(1.0, 5.0*scaling);
   
   auto& camera = mainCanvas->get_camera();
   //check if camera dimensions are correct
   
   //cout << 100.0/screenArea << ", " << absoluteLength << endl;
   

   map<const OSMID, string> wayRoute;

   //set line settings
   g->set_line_width(lineWidth);
   g->set_line_cap(ezgl::line_cap::round);

   vector<pair<int, int>> cells = getCurrentRenderCells(&camera,kRenderCellSize,minPoints);
   //if number of cells is low, use cell rendering method
   cout << "cells: " << cells.size() << endl;
   bool useCellRendering = cells.size() <= kMaximumRenderCells; //use render cells to cull features
   cout << "cell rendering: " << useCellRendering << endl;
   if(useCellRendering){
      vector<MapFeature*> cellFeatures = getFeaturesFromCells(cells, cellMap);
      for(int i = 0; i < cellFeatures.size(); i++)
      {
         
         MapFeature* feature = cellFeatures[i];
         g->set_color({0, 0, 0, 255});
         g->set_line_width(lineWidth);
         drawFeature(feature,g,screenArea,lineWidth,scaling,camera);
      }
   }
   else //if number of cells is too high, directly render each element
   {
      auto layer = mapFeatures.layerMap.begin();
      while(layer != mapFeatures.layerMap.end())
      {
         vector<MapFeature*> cellFeatures = *(layer->second);
         for(int i = 0; i < cellFeatures.size(); i++)
         {
            MapFeature* feature = cellFeatures[i];
            //run bounding box checks
            if(feature->drawType == POINT && !camera.get_world().contains((*feature->points)[0])){continue;} //check if point is inside world rect
            if(feature->drawType != POINT && !intersecting(camera.get_world(),feature->bounds)){continue;} //check if bounds for camera and feaure are intersecting
            g->set_color({0, 0, 0, 255});
            g->set_line_width(lineWidth);
            drawFeature(feature,g,screenArea,lineWidth,scaling,camera);
         }
         layer++;
      }
   }
   
   
   //set text display settings
   g->set_font_size(10);
   g->set_horiz_justification(ezgl::justification::center); //horizontally align text position with center of text
   g->set_vert_justification(ezgl::justification::center); //vertically align text position with center of text
   //g->format_font("sans-serif",ezgl::font_slant::normal,ezgl::font_weight::bold);
   g->set_color({0, 0, 0, 200}); //print transluscent black text
   //iterate through all labels
   for(auto label = staticLabels.begin(); label != staticLabels.end(); label++)
   {
      g->draw_text(label->position, label->text, label->feature->bounds.width()/1.5, label->feature->bounds.height()/1.5);
   }
}

double getScaleFactor()
{
   return mainCanvas->get_camera().get_world_scale_factor().x;
}

void mapRenderCells()
{
   cout << "mapping render cells" << endl;
   cout << "..." << endl;
   auto layer = mapFeatures.layerMap.begin();
   cout << "got layer" << endl;
   double originY = LatToY(minPoints.latitude());
   double originX = LonToX(minPoints.longitude());

   while(layer != mapFeatures.layerMap.end())
   {
      for(int i = 0; i < layer->second->size(); i++)
      {
         
         MapFeature* feature = (*(layer->second))[i];
         vector<pair<int, int>> cells;
         switch(feature->drawType)
         {
            case POINT: {
               cout << "adding point" << endl;
               //only print points if they are highlighted
               cells.push_back(getRenderCell(originX,originY,kRenderCellSize,(*feature->points)[0]));
               break;
            }
            case LINE: {
               //calculate the transparency of a street based on its length
               ezgl::rectangle& bounds = feature->bounds;
               cells = getBoundedRenderCells(bounds,kRenderCellSize,minPoints);
               break;
            }
            case POLYGON: {
               ezgl::rectangle& bounds = feature->bounds;
               cells = getBoundedRenderCells(bounds,kRenderCellSize,minPoints);
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

void clickedOnFeature(MapFeature* clicked, ezgl::application* app) //function to manage clicks on a specific MapFeature
{
   mapFeatures.highlightFeature(clicked);
   auto g = app->get_renderer();
   if(clicked->featureType == FEATURE) //clicked on feature
   {
      OSMID osmid = getFeatureOSMID(clicked->dataIndex);
      cout << "clicked: " << getFeatureName(clicked->dataIndex) << endl;
   }
   if(clicked->featureType == ROAD && clicked->dataIndex < getNumStreetSegments()) //clicked on street segment
   {
      StreetSegmentInfo segmentInfo = getStreetSegmentInfo(clicked->dataIndex);
      cout << "clicked: sID = " << getStreetSegmentInfo(clicked->dataIndex).streetID << endl;
      
   }
   mainCanvas->redraw();
}

void on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y)
{
   if(mapFeatures.layerMap.size() == 0){return;} //if there are no features, skip click detection
   cout << "click at " << x << ", " << y << endl;
   
   auto& camera = mainCanvas->get_camera();

   map<const OSMID, string> wayRoute;

   //to prioritize higher layers in click detection, run through features backwards
   auto layer = mapFeatures.layerMap.end();
   layer--;

   double minDistance = DBL_MAX;
   MapFeature* clicked = nullptr;
   do{
      //get closest feature
      for(int i = 0; i < layer->second->size(); i++)
      {
         MapFeature* feature = (*(layer->second))[i];
         if(!feature->wasRendered){continue;} //dont register non-rendered/invisible objects as clickable
         ezgl::rectangle bounds = feature->bounds;
         //for all features, compare distance from points
         for(auto point = feature->points->begin(); point != feature->points->end(); point++){
            double xDist = point->x - x;
            double yDist = point->y - y;
            double distanceToMouse = sqrt(xDist*xDist + yDist*yDist);
            if(distanceToMouse < minDistance)
            {
               clicked = feature;
               minDistance = distanceToMouse;
            }
         }
         //for polygons, also compare distance from center of bounding box
         if(feature->drawType == POLYGON)
         {
            double xDist = feature->bounds.center_x() - x;
            double yDist = feature->bounds.center_y() - y;
            double distanceToMouse = sqrt(xDist*xDist + yDist*yDist);
            if(distanceToMouse < minDistance)
            {
               clicked = feature;
               minDistance = distanceToMouse;
            }
         }
      }
      layer--;
   }while(layer != mapFeatures.layerMap.begin());

   if(clicked != nullptr)
   {
      clickedOnFeature(clicked, app); //run click events on the clicked feature  
   }
}

void loadFeatures()
{
   double minimumLatitude = DBL_MAX; //smallest latitude in city
   double minimumLongitude = DBL_MAX; //smallest longitude in city
   double maximumLatitude = -DBL_MAX; //largest latitude in city
   double maximumLongitude = -DBL_MAX; //largest longitude in city
   //go through all features to find largest and smallest latlon points
   for(IntersectionIdx i = 0; i < getNumIntersections(); i++)
   {
      LatLon point = getIntersectionPosition(i); //get position of map feature
      updateMaximumLatLon(minimumLatitude,minimumLongitude,maximumLatitude,maximumLongitude,point); //compare/replace max and min to current values
   }

   minPoints = LatLon(minimumLatitude, minimumLongitude); //consolidate minimum latitude and longitude in latlon object
   maxPoints = LatLon(maximumLatitude, maximumLongitude); //consolodate maximum latitude and longitude in latlon object
   setLatLonBounds(minPoints, maxPoints); //set constant for calculations in renderHelper

   //Cache street lengths
   for(StreetIdx streetIndex = 1; streetIndex < getNumStreets(); streetIndex++)
   {
      double streetLength = findStreetLength(streetIndex); //get length of street
      streetLengthCache.push_back(streetLength); //cache street length
   }
   
   //Add intersection features
   for(IntersectionIdx intersectionIndex = 0; intersectionIndex < getNumIntersections(); intersectionIndex++)
   {
      vector<ezgl::point2d>* points = new vector<ezgl::point2d>; //create vector to add points
      ezgl::point2d translatedPoint = LatLonToPoint2D(getIntersectionPosition(intersectionIndex)); //get intersection coor
      points->push_back(translatedPoint); //add singular point to point list
      MapFeature* mapFeature = new MapFeature(10,points,kColorHighlight,POINT,MapFeatureType::INTERSECTION,(int)intersectionIndex); //add intersection as map feature
      mapFeatures.addFeature(mapFeature);
   }

   //cout << "loading segments: " << streetSegmentPoints.size() << endl;
   for(StreetSegmentIdx streetSegmentIndex = 0; streetSegmentIndex < getNumStreetSegments(); streetSegmentIndex++)
   {
      vector<LatLon> curvePoints = *getStreetSegmentPoints(streetSegmentIndex); //get cached curve points for street segment
      vector<ezgl::point2d>* points = new vector<ezgl::point2d>;
      for(int pointIndex = 0; pointIndex < curvePoints.size(); pointIndex++)
      {
         LatLon curvePoint = curvePoints[pointIndex];
         ezgl::point2d translatedPoint = LatLonToPoint2D(curvePoint);
         points->push_back(translatedPoint);
      }
      auto segmentInfo = getStreetSegmentInfo(streetSegmentIndex); //street segment info
   
      ezgl::color roadColor = kColorBasicRoad; //colour of feature (default is building) 

      //if road is oneway, indicate it using segment info
      if(segmentInfo.oneWay)
      {
         roadColor = kColorOnewayRoad;
      }
      
      MapFeature* mapFeature = new MapFeature(1,points,roadColor,LINE,MapFeatureType::ROAD,(int)streetSegmentIndex);  //create map featur object
      mapFeatures.addFeature(mapFeature);

      //set street id
      StreetIdx streetID = segmentInfo.streetID; //get street ID
      mapFeature->streetID = streetID; //set street id of feature to calculated street id
      //check if street is a major street
      if(streetID > 0) //street ID 0 can never be a major road as it is the default ID for minor roads
      {  
         double streetLength = streetLengthCache[streetID]; //get street length
         bool majorRoad = segmentInfo.speedLimit > 18; //if road is long, it is a major road
         if(majorRoad)
         {
            mapFeature->major = majorRoad;
            mapFeature->color = kColorMajorRoad;
            if(segmentInfo.oneWay)
            {
               mapFeature->color = kColorOnewayMajorRoad;
            }
         }
      }
   }

   for(FeatureIdx featureIndex = 0; featureIndex < getNumFeatures(); featureIndex++)
   {
      vector<ezgl::point2d>* points = new vector<ezgl::point2d>; //create array to store cartesian points
      //loop through all latlon points in feature
      bool closed = (getFeaturePoint(featureIndex, 0) == getFeaturePoint(featureIndex, getNumFeaturePoints(featureIndex) - 1));

      for(int pointIndex = 0; pointIndex < getNumFeaturePoints(featureIndex); pointIndex++)
      {
         LatLon featurePoint = getFeaturePoint(featureIndex, pointIndex); //get latlon point
         points->push_back(LatLonToPoint2D(featurePoint)); //convert latlon point to cartesion point and add to vector
      }
      //if feature is invalid, deallocate space and skip
      if(points->size() <= 1)
      {
         delete points;
         continue;
      }

      int layer = -2; //layer of feature (default is -2)
      double drawArea = findFeatureArea(featureIndex); //area of feature to use to decide whether to render the feature at a specific zoom
      ezgl::color featureColor = kColorBuilding; //colour of feature (default is building) 
      FeatureType featureType = getFeatureType(featureIndex); //get feature type
      //change colour and layer based on feature type
      switch(featureType)
      {
         case UNKNOWN:{
            featureColor = {217, 217, 217, 255};
            layer = -10;
            break;
         }
         case PARK:{
            featureColor = kColorScrub;
            layer = -7;
            break;
         }
         case BEACH:{
            featureColor = kColorBeach;
            layer = -6;
            break;
         }
         case LAKE:{
            featureColor = kColorWater;
            layer = -9;
            break;
         }
         case RIVER:{
            featureColor = kColorWater;
            layer = -3;
            break;
         }
         case ISLAND:{
            featureColor = kColorIsland;
            layer = -8;
            break;
         }
         case BUILDING:{
            featureColor = kColorBuilding;
            layer = 2;
            drawArea = 50; //set area so all buildings have the same render distance
            break;
         }
         case GREENSPACE:{
            featureColor = kColorWood;
            layer = -5;
            break;
         }
         case GOLFCOURSE:{
            featureColor = kColorBeach;
            layer = -4;
            break;
         }
         case STREAM:{
            featureColor = kColorWater;
            layer = -2;
            break;
         }
      }
      
      //default draw type is a polygon
      DrawType drawType = POLYGON;
      //if feature is not closed, then it is a line
      if(!closed){drawType = LINE;}

      

      MapFeature* mapFeature = new MapFeature(layer,points,featureColor,drawType,MapFeatureType::FEATURE,(int)featureIndex); //create map feature object
      mapFeatures.addFeature(mapFeature); //add map feature object to array
      //add text label for feature name
      string featureName = getFeatureName(featureIndex);
      if(featureName[0] != '<') //dont print text that is not meant to be displayed (denoted by being wrapped in '<>')
      {
         staticLabels.push_back({featureName,mapFeature->bounds.center(),mapFeature}); //create static text structure and add to array for rendering
      }
   }

   cout << "starting to map render cells" << endl;
   mapRenderCells();
   cout << endl;
}





void drawMap() {
   // Set up the ezgl graphics window and hand control to it, as shown in the 
   // ezgl example program. 
   // This function will be called by both the unit tests (ece297exercise) 
   // and your main() function in main/src/main.cpp.
   // The unit tests always call loadMap() before calling this function
   // and call closeMap() after this function returns.
   ezgl::application::settings settings; //initialize settings
   settings.main_ui_resource = "libstreetmap/resources/main.ui"; //load ui data
   settings.window_identifier = "MainWindow"; //setup window
   settings.canvas_identifier = "MainCanvas"; //setup canvas
   ezgl::application application(settings); //create application
   initial_world = ezgl::rectangle(LatLonToPoint2D(minPoints),LatLonToPoint2D(maxPoints));
   mainCanvas = application.add_canvas("MainCanvas", draw_main_canvas, initial_world);
   application.mouse_press_callback;
   
   //create window
   application.run(nullptr,on_mouse_click,nullptr,nullptr);


}
