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
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "ezgl/camera.hpp"

using namespace std;

static ezgl::rectangle initial_world{{0, 0}, 1150, 1150}; //rendered canvas
static FeatureList mapFeatures;
 


void draw_main_canvas(ezgl::renderer *g)
{
   ezgl::rectangle rect(initial_world.bottom_left(),initial_world.top_right());
   g->draw_rectangle(rect);
   auto layer = mapFeatures.layerMap.begin();
   while(layer != mapFeatures.layerMap.end()){

      for(int i = 0; i < layer->second->size(); i++)
      {
         MapFeature* feature = (*(layer->second))[i];
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
               for(int i = 0; i < feature->points->size() - 1; i++)
               {
                  g->draw_line((*feature->points)[i],(*feature->points)[i+1]);
               }
               break;
            }
            //cout << "Printing at " << origin.x << ", " << origin.y << endl;
         }
      }
      layer++;
   }
}

void updateMaximumLatLon(double& minimumLatitude, double& minimumLongitude, double& maximumLatitude,double& maximumLongitude, LatLon point)
{
   //check if latitude is higher than recorded maximum latitude
   if(point.latitude() > maximumLatitude)
   {
      maximumLatitude = point.latitude(); //replace recorded maximum with new value
   }
   //check if longitude is higher than recorded maximum latitude
   cout << point.longitude() << " <> " << maximumLongitude << endl;
   if(point.longitude() > maximumLongitude)
   {
      maximumLongitude = point.longitude(); //replace recorded maximum with new value
      cout << "new max: " << maximumLongitude << endl;
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

   LatLon minPoints = LatLon(minimumLatitude, minimumLongitude); //consolidate minimum latitude and longitude in latlon object
   LatLon maxPoints = LatLon(maximumLatitude, maximumLongitude); //consolodate maximum latitude and longitude in latlon object
   double scale = 1150;

   for(IntersectionIdx i = 0; i < getNumIntersections(); i++)
   {
      vector<ezgl::point2d>* points = new vector<ezgl::point2d>;
      ezgl::point2d translatedPoint = LatLonToPoint2D(maxPoints, minPoints, getIntersectionPosition(i),scale);
      points->push_back(translatedPoint);
      MapFeature* mapFeature = new MapFeature(0,points,POINT);
      mapFeatures.addFeature(mapFeature);
   }


   //cout << "loading segments: " << streetSegmentPoints.size() << endl;
   for(StreetSegmentIdx i = 0; i < getNumStreetSegments(); i++)
   {
      vector<LatLon> curvePoints = *getStreetSegmentPoints(i); //get cached curve points for street segment
      vector<ezgl::point2d>* points = new vector<ezgl::point2d>;
      for(int pointIndex = 0; pointIndex < curvePoints.size(); pointIndex++)
      {
         cout << "starting: " << curvePoints.size() << endl;
         LatLon curvePoint = curvePoints[pointIndex];
         ezgl::point2d translatedPoint = LatLonToPoint2D(maxPoints, minPoints,curvePoint,scale);
         points->push_back(translatedPoint);
         cout << "done!" << endl;
      }
      MapFeature* mapFeature = new MapFeature(0,points,LINE);
      mapFeatures.addFeature(mapFeature);
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

   application.add_canvas("MainCanvas", draw_main_canvas, initial_world);

   //create window
   application.run(nullptr,nullptr,nullptr,nullptr);


}
