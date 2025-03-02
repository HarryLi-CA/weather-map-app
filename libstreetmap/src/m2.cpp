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
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "nodeIDHelper.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "ezgl/camera.hpp"

using namespace std;

static ezgl::rectangle initial_world{{0, 0}, 1150, 1150}; //rendered canvas
static FeatureList mapFeatures;
static ezgl::canvas* mainCanvas;
LatLon minPoints;
LatLon maxPoints;
map<const OSMID, const OSMRelation *> osmRelations;
//default colour constants for general objects
const ezgl::color kColourDefaultGround = {235, 235, 235, 255}; 
//default colour constants for features
const ezgl::color kColorScrub = {212, 247, 178, 255};
const ezgl::color kColorWood = {138, 191, 134, 255};
const ezgl::color kColorBuilding = {166, 166, 166, 255};
const ezgl::color kColorWater = {147, 194, 230, 255};
const ezgl::color kColorBeach = {242,242,184, 255};
const ezgl::color kColorIsland = {238, 242, 213, 255};
//default colour constants for roads
const ezgl::color kColorBasicRoad = {70, 70, 70, 255};
const ezgl::color kColorOnewayRoad = {219, 118, 118, 255};


void draw_main_canvas(ezgl::renderer *g)
{
   ezgl::rectangle rect(initial_world.bottom_left(),initial_world.top_right());
   g->set_color(kColourDefaultGround);
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

   //cout << 100.0/screenArea << ", " << absoluteLength << endl;
   g->set_line_width(lineWidth);

   map<const OSMID, string> wayRoute;

   while(layer != mapFeatures.layerMap.end()){
      for(int i = 0; i < layer->second->size(); i++)
      {
         MapFeature* feature = (*(layer->second))[i];
         g->set_color({0, 0, 0, 255});
         switch(feature->drawType)
         {
            case POINT: {
               ezgl::point2d origin = (*(feature->points))[0];
               origin = {origin.x + initial_world.bottom_left().x, origin.y + initial_world.bottom_left().y};
               ezgl::rectangle featureRect({origin.x - 1, origin.y - 1},{origin.x + 1, origin.y + 1});
               g->fill_rectangle(featureRect);
               break;
            }
            case LINE: {
               
               //dont process objects that are not in the camrea
               if(!intersecting(mainCanvas->get_camera().get_world(),feature->bounds))
               {
                  break;
               }
               //calculate the transparency of a street based on its length
               ezgl::rectangle& bounds = feature->bounds;
               ezgl::rectangle screen_rect(camera.world_to_screen(bounds.bottom_left()),camera.world_to_screen(bounds.top_right()));
               if(screen_rect.area() < 10){break;} 

               double streetAlpha = 255;

               if(streetAlpha <= 0) {break;} //if the street is invisible, skip rendering to save processing time
               
               g->set_color(feature->color);
               for(int i = 0; i < feature->points->size() - 1; i++)
               {
                  g->draw_line((*feature->points)[i],(*feature->points)[i+1]);
               }
               break;
            }
            case POLYGON: {

               //dont process objects that are not in the camrea
               if(!intersecting(mainCanvas->get_camera().get_world(),feature->bounds))
               {
                  break;
               }
               //dont render small features
               const ezgl::rectangle& bounds = feature->bounds;
               
               const ezgl::rectangle screen_rect(camera.world_to_screen(bounds.bottom_left()),camera.world_to_screen(bounds.top_right()));
               if(screen_rect.area() < 10){break;} 
               
               //cout << feature->maximumScreenLength << "/"<< screenArea <<" = "<< screenSpace << endl;
               g->set_color(feature->color);
               g->fill_poly(*(feature->points));
               //g->draw_rectangle(feature->bounds);
            }
            //cout << "Printing at " << origin.x << ", " << origin.y << endl;
         }
      }
      layer++;
   }
}

double getScaleFactor()
{
   return mainCanvas->get_camera().get_world_scale_factor().x;
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

   //Add intersection features
   for(IntersectionIdx i = 0; i < getNumIntersections(); i++)
   {
      //vector<ezgl::point2d>* points = new vector<ezgl::point2d>; //create vector to add points
      //ezgl::point2d translatedPoint = LatLonToPoint2D(getIntersectionPosition(i)); //get intersection coor
      //points->push_back(translatedPoint);
      //MapFeature* mapFeature = new MapFeature(0,points,POINT,0,kColorRoad);
      //mapFeatures.addFeature(mapFeature);
   }

   //cout << "loading segments: " << streetSegmentPoints.size() << endl;
   for(StreetSegmentIdx i = 0; i < getNumStreetSegments(); i++)
   {
      vector<LatLon> curvePoints = *getStreetSegmentPoints(i); //get cached curve points for street segment
      vector<ezgl::point2d>* points = new vector<ezgl::point2d>;
      for(int pointIndex = 0; pointIndex < curvePoints.size(); pointIndex++)
      {
         LatLon curvePoint = curvePoints[pointIndex];
         ezgl::point2d translatedPoint = LatLonToPoint2D(curvePoint);
         points->push_back(translatedPoint);
      }
      auto segmentInfo = getStreetSegmentInfo(i); //street segment info
   
      ezgl::color roadColor = kColorBasicRoad; //colour of feature (default is building) 

      //if road is oneway, indicate it using segment info
      if(segmentInfo.oneWay)
      {
         roadColor = kColorOnewayRoad;
      }
      
      MapFeature* mapFeature = new MapFeature(1,points,roadColor,LINE,MapFeatureType::ROAD,(int)i);  //create map featur object
      mapFeatures.addFeature(mapFeature);
   }

   for(FeatureIdx i = 0; i < getNumFeatures(); i++)
   {
      vector<ezgl::point2d>* points = new vector<ezgl::point2d>; //create array to store cartesian points
      //loop through all latlon points in feature
      bool closed = (getFeaturePoint(i, 0) == getFeaturePoint(i, getNumFeaturePoints(i) - 1));

      for(int pointIndex = 0; pointIndex < getNumFeaturePoints(i); pointIndex++)
      {
         LatLon featurePoint = getFeaturePoint(i, pointIndex); //get latlon point
         points->push_back(LatLonToPoint2D(featurePoint)); //convert latlon point to cartesion point and add to vector
      }
      //if feature is invalid, deallocate space and skip
      if(points->size() <= 1)
      {
         delete points;
         continue;
      }

      int layer = -2; //layer of feature (default is -2)
      double drawArea = findFeatureArea(i); //area of feature to use to decide whether to render the feature at a specific zoom
      ezgl::color featureColor = kColorBuilding; //colour of feature (default is building) 
      FeatureType featureType = getFeatureType(i); //get feature type
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
      MapFeature* mapFeature = new MapFeature(layer,points,featureColor,drawType,MapFeatureType::FEATURE,(int)i); //create map feature object
      mapFeatures.addFeature(mapFeature); //add map featue object to array
   }
   cout << endl;
}

void clickedOnFeature(MapFeature* clicked)
{
   if(clicked->featureType == FEATURE) //clicked on feature
   {
      OSMID osmid = getFeatureOSMID(clicked->dataIndex);
      cout << "clicked: " << getFeatureName(clicked->dataIndex) << endl;
   }
   if(clicked->featureType == ROAD) //clicked on street segment
   {
      StreetSegmentInfo segmentInfo = getStreetSegmentInfo(clicked->dataIndex);
      cout << "clicked: " << getStreetName(getStreetSegmentInfo(clicked->dataIndex).streetID) << endl;
   }
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
   MapFeature* clicked = nullptr;
   do{
      for(int i = 0; i < layer->second->size(); i++)
      {
         MapFeature* feature = (*(layer->second))[i];
         ezgl::rectangle bounds = feature->bounds;
         if(feature->drawType == LINE || feature->drawType == POLYGON)
         {
            //if mouse is in map feature bounding box, reister that map feature as clicked (and exit loop)
            if(bounds.contains(x, y)){
               clicked = feature;
               break;
            }
         }
      }
      layer--;
   }while(layer != mapFeatures.layerMap.begin());

   if(clicked != nullptr)
   {
      
   }
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
