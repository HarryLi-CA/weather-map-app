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
#include "OpenWeatherHelper.h"
#include <vector>
#include <map>
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "nodeIDHelper.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "ezgl/camera.hpp"
#include <set>
#include <ctime>

using namespace std;

static ezgl::rectangle initial_world{{0, 0}, 1920, 1080}; //rendered canvas
static FeatureList mapFeatures; //list of map features to draw
static ezgl::canvas* mainCanvas; //application canvas
LatLon minPoints; //bottom left LatLon position
LatLon maxPoints; //top right LatLon position
vector<StaticName> staticLabels; //list of static text objects to draw
vector<MapFeature*> intersectionFeatures; //vector that takes in intersection index and return correpsonding map feature
vector<StreetIdx> streetLengthCache; //stored value of all loaded street lengths;
map<pair<int, int>, vector<MapFeature*>*> cellMap; //map of all render cells to list of features
int numberOfStreets = 0; //save number of streets for fast access
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
const ezgl::color kColorBasicRoad = {180, 180, 180, 255};
const ezgl::color kColorOnewayRoad = {219, 118, 118, 255};
const uint_fast8_t kColorOnewayIndicatorBrightness = 255;
const ezgl::color kColorMajorRoad = {0, 0, 200, 255};
const ezgl::color kColorOnewayMajorRoad = {148,0,211, 255};
const ezgl::color kColorStreetName = {255, 255, 255, 255};
//scaling values for window resizing
double initialWidth = -1;
double initialHeight = -1;
//define rendering constants
const double kOnewayIndicatorSize = 6.0; //size from center to vertex of indicator triangle
const int kMinimumPixelsToDraw = 100; //minimum pixels a feature should take up on the screen to be drawn that cycle
const double kMinimumMajorRoadLength = 5000; //minimum length for a road to be considered a major road
const double kRenderCellSize = 300; //size of each render cell
const int kMaximumRenderCells = 50000; //maximum number of render cells before switching to direct rendering
const double kMaximumBuildingScaling = 2; //maximum number of meters per pixel to start drawing buildings
const bool enableCellRendering = true; //enable cell rendering method (possibly buggy)
const double kRoadWidth = 7.5; //width of roads
const double TILERATIO = 0.000005805;
LatLonBounds currentScope; // scope of city map
vector<MapTileData> mapTiles;
WeatherData currentWeather; // weather including sunset and sunrise in epoch time

//Stores the intersectionIdxs of the previously searched intersections
vector<IntersectionIdx> prevSearchedIntersectionIdxs = {};
//Stores previously clicked intersectionIdx;
IntersectionIdx prevClickedIntersection = -1;
//Stores the map button id along with its path and a application pointer to send into the map changing callback
pair<string,pair<string,ezgl::application*>> mapNames[19] = { {"BeijingBtn",{"/cad2/ece297s/public/maps/beijing_china.streets.bin",nullptr}},
                                                            {"BeirutBtn",{"/cad2/ece297s/public/maps/beirut_lebanon.streets.bin",nullptr}},
                                                            {"BerlinBtn",{"/cad2/ece297s/public/maps/berlin_germany.streets.bin",nullptr}},
                                                            {"BostonBtn",{"/cad2/ece297s/public/maps/boston_usa.streets.bin",nullptr}},
                                                            {"CapeTownBtn",{"/cad2/ece297s/public/maps/cape-town_south-africa.streets.bin",nullptr}},
                                                            {"GoldenHorseshoeBtn",{"/cad2/ece297s/public/maps/golden-horseshoe_canada.streets.bin",nullptr}},
                                                            {"HamiltonBtn",{"/cad2/ece297s/public/maps/hamilton_canada.streets.bin",nullptr}},
                                                            {"HongKongBtn",{"/cad2/ece297s/public/maps/hong-kong_china.streets.bin",nullptr}},
                                                            {"IcelandBtn",{"/cad2/ece297s/public/maps/iceland.streets.bin",nullptr}},
                                                            {"InterlakenBtn",{"/cad2/ece297s/public/maps/interlaken_switzerland.streets.bin",nullptr}},
                                                            {"LondonBtn",{"/cad2/ece297s/public/maps/london_england.streets.bin",nullptr}},
                                                            {"NewDehliBtn",{"/cad2/ece297s/public/maps/new-delhi_india.streets.bin",nullptr}},
                                                            {"NewYorkBtn",{"/cad2/ece297s/public/maps/new-york_usa.streets.bin",nullptr}},
                                                            {"RioDeJaneiroBtn",{"/cad2/ece297s/public/maps/rio-de-janeiro_brazil.streets.bin",nullptr}},
                                                            {"SaintHelenaBtn",{"/cad2/ece297s/public/maps/saint-helena.streets.bin",nullptr}},
                                                            {"SingaporeBtn",{"/cad2/ece297s/public/maps/singapore.streets.bin",nullptr}},
                                                            {"TehranBtn",{"/cad2/ece297s/public/maps/tehran_iran.streets.bin",nullptr}},
                                                            {"TokyoBtn",{"/cad2/ece297s/public/maps/tokyo_japan.streets.bin",nullptr}},
                                                            {"TorontoBtn",{"/cad2/ece297s/public/maps/toronto_canada.streets.bin",nullptr}}};
//Stores the filter button id as well as the WeatherLayer it is asciated to
pair<string,pair<WeatherLayer,ezgl::application*>> filterNames[6] = { {"SnowBtn",{SNOW,nullptr}},
                                                                     {"TemperatureBtn",{TEMPERATURE,nullptr}},
                                                                     {"PrecipitationBtn",{PRECIPITATION,nullptr}},
                                                                     {"CloudsBtn",{CLOUDS,nullptr}},
                                                                     {"HumidityBtn",{HUMIDITY,nullptr}},
                                                                     {"WindBtn",{WIND,nullptr}}};

WeatherLayer currWeatherLayer = NOFILTER;  
std::time_t currentTime = std::time(nullptr);                            

// void drawTriangle(ezgl::renderer *g, ezgl::point2d center, ezgl::point2d direction, ezgl::color color, double distanceFromCenter)
// {
//    double angle = atan2(direction.y, direction.x);
//    //draw triangle based on initial angle
//    g->set_color(color);
//    g->fill_poly({
//       {center.x + distanceFromCenter*cos(angle), center.y + distanceFromCenter*sin(angle)},
//       {center.x + distanceFromCenter*cos(angle+2*M_PI/3.0), center.y + distanceFromCenter*sin(angle+2*M_PI/3.0)},
//       {center.x + distanceFromCenter*cos(angle+4*M_PI/3.0), center.y + distanceFromCenter*sin(angle+4*M_PI/3.0)},
//    });
// }

void drawFeature(MapFeature *feature, ezgl::renderer *g, double lineWidth, double scaling, double realWorldScaling, ezgl::camera camera)
{  
   switch(feature->drawType)
      {
         case POINT: {
            //only print points if they are highlighted
            feature->wasRendered = true; 
            if(!feature->highlight){break;}
            feature->wasRendered = false;
            
            ezgl::point2d origin = (*(feature->points))[0];
            g->set_color(feature->color);
            g->fill_arc(origin,7.0/scaling,0.0,360.0);
            
            feature->wasRendered = true; //feature was fully rendered this cycle
            break;
         }
         case LINE: {
            double lineWidthScale = 1; //scale of line width used to render line
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
               lineWidthScale = 2;
               g->set_line_width(lineWidth*lineWidthScale);
            }
            for(int i = 0; i < feature->points->size() - 1; i++)
            {
               g->draw_line((*feature->points)[i],(*feature->points)[i+1]);
            }
            //get direction of road
            ezgl::point2d from = (*feature->points)[0]; //first position in the street
            ezgl::point2d to = (*feature->points)[feature->points->size() - 1]; //last position in the street
            ezgl::point2d dir = {to.x - from.x, to.y - from.y}; //direction vector for street
            

            //draw arrow if street is directional
            if(feature->directional && lineWidth*lineWidthScale > kOnewayIndicatorSize)
            {
               
               double magnitude = sqrt(dir.x*dir.x + dir.y*dir.y); //magnitude of dir
               if(magnitude > 0){
                  ezgl::color indicatorColour = {kColorOnewayIndicatorBrightness,kColorOnewayIndicatorBrightness,kColorOnewayIndicatorBrightness,200};
                  ezgl::point2d midpoint = (*feature->points)[feature->points->size()/2]; //approximate mid point to be the point halfway along the list of points
                  //draw triangle on road
                  drawTriangle(g,midpoint,dir,indicatorColour,kOnewayIndicatorSize/scaling);
               }
            }
            //draw street name if applicable
            if(feature->streetID < numberOfStreets && feature->streetID > 0){
               
               int longestIndex = feature->longestPointSegmentIndex;
               //get longest segment points
               ezgl::point2d fromPoint = (*feature->points)[longestIndex];
               ezgl::point2d toPoint = (*feature->points)[longestIndex+1];

               //get point in the middle of longest segment
               ezgl::point2d midPoint = midpoint(fromPoint, toPoint);
               //get displacement vector between points
               ezgl::point2d delta = {toPoint.x - fromPoint.x, toPoint.y - fromPoint.y};
               double angle = (atan2(delta.y, delta.x))/kDegreeToRadian; //angle from horizontal axis (counter clockwise) of street segment
               if(angle > 90){angle -= 180;}
               if(angle < -90){angle += 180;}
               string streetName = getStreetName(feature->streetID);

               //if(angle > 180){angle -= 180;} //unflip angles that point downward

               //roate and draw text
               g->set_color(kColorStreetName);
               g->set_text_rotation(angle);
               g->draw_text(midPoint,streetName,feature->longestSegmentLength,lineWidth*lineWidthScale);
            }
            feature->wasRendered = true; //feature was fully rendered this cycle
            break;
         }
         case POLYGON: {
            feature->wasRendered = false;
            //check scaling requirements
            if(realWorldScaling > feature->maximumScaling){break;} //if current scaling is more than max scaling, skip feature
            //dont render small features
            const ezgl::rectangle& bounds = feature->bounds;
            
            const ezgl::rectangle screen_rect(camera.world_to_screen(bounds.bottom_left()),camera.world_to_screen(bounds.top_right()));
            if(!feature->highlight && screen_rect.area() < kMinimumPixelsToDraw){break;} 
         
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
         default: {
            break;
         }
      }

      //reset temporary settings
      g->set_text_rotation(0);
}

void draw_main_canvas(ezgl::renderer *g)
{
   //rect value to draw to screen
   ezgl::rectangle rect(initial_world.bottom_left(),initial_world.top_right());
   g->set_color(kColorDefaultGround);
   g->fill_rectangle(rect);

   double pixelLength = mainCanvas->get_camera().get_screen().width();
   double screenLength = mainCanvas->get_camera().get_world().width();
   double scaling = pixelLength/screenLength;
   double worldToScreenScale = screenLength/pixelLength;

   int lineWidth = max(1.0, kRoadWidth*scaling);

   auto& camera = mainCanvas->get_camera();
   ezgl::rectangle cameraRect = camera.get_world();  

   double rectWidth = cameraRect.width()/(camera.get_screen().width()/mainCanvas->width()); //caculate world width of canvas
   double rectHeight = cameraRect.height()/(camera.get_screen().height()/mainCanvas->height()); //calculate world height of canvas
   cameraRect = ezgl::rectangle(
      {cameraRect.center_x() - rectWidth/2.0, cameraRect.center_y() - rectHeight/2.0},
      {cameraRect.center_x() + rectWidth/2.0, cameraRect.center_y() + rectHeight/2.0}
   );
   //check if camera dimensions are correct
   
   //cout << 100.0/screenArea << ", " << absoluteLength << endl;
   

   map<const OSMID, string> wayRoute;

   //set line settings
   g->set_line_width(lineWidth);
   g->set_line_cap(ezgl::line_cap::round);
   g->set_font_size(10);
   g->set_horiz_justification(ezgl::justification::center); //horizontally align text position with center of text
   g->set_vert_justification(ezgl::justification::center); //vertically align text position with center of text
   g->format_font("Arial",ezgl::font_slant::normal,ezgl::font_weight::normal);
   bool useCellRendering = false;
   vector<pair<int, int>> cells;
   if(enableCellRendering){
      //get cells currently on screen
      cells = getCurrentRenderCells(&camera,kRenderCellSize,minPoints);
      //if number of cells is low, use cell rendering method
      useCellRendering = cells.size() <= kMaximumRenderCells; //use render cells to cull features
      cout << cells.size() << endl;
   }
   
   if(useCellRendering){
      vector<MapFeature*> cellFeatures = getFeaturesFromCells(cells, cellMap);
      for(int i = 0; i < cellFeatures.size(); i++)
      {
         MapFeature* feature = cellFeatures[i];
         g->set_color({0, 0, 0, 255});
         g->set_line_width(lineWidth);
         drawFeature(feature,g,lineWidth,scaling,worldToScreenScale,camera);
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
            if(feature->drawType == POINT && !cameraRect.contains((*feature->points)[0])){continue;} //check if point is inside world rect
            if(feature->drawType != POINT && !intersecting(cameraRect,feature->bounds)){continue;} //check if bounds for camera and feaure are intersecting
            g->set_color({0, 0, 0, 255});
            g->set_line_width(lineWidth);
            drawFeature(feature,g,lineWidth,scaling,worldToScreenScale,camera);
         }
         layer++;
      }
   }

   //draw weather overlay
   //getMapTilesFromBounds();
   if(mapTiles.size()>0){
   double tileSize = LonToX(mapTiles[0].tileBounds.max.longitude()) - LonToX(mapTiles[0].tileBounds.min.longitude());
   double zoom = getScaleFactor();
   for (size_t i = 0; i < mapTiles.size(); i++)
   {
      const char* surfURL = mapTiles[i].url.c_str();
      ezgl::surface* currentSurface = g->load_png(surfURL);
      ezgl::point2d minPoint2d= LatLonToPoint2D(mapTiles[i].tileBounds.min);
      ezgl::point2d maxPoint2d= LatLonToPoint2D(mapTiles[i].tileBounds.max);

      //cout<< "ratio: " << 1/(tileSize/zoom) <<endl;TILESIZE
      double metersPerPixel = tileSize/TILEWIDTH;
      double screenMperP = screenLength/pixelLength;
      double targetLength = tileSize/screenMperP;
      double scalingFactor = targetLength/TILEWIDTH;

      g->draw_surface(currentSurface, {(minPoint2d.x+maxPoint2d.x)/2,(minPoint2d.y+maxPoint2d.y)/2},scalingFactor);
      g->free_surface(currentSurface);
   }
   }
   

   //set text display settings
   
   g->set_color({0, 0, 0, 200}); //print transluscent black text
   //iterate through all labels
   for(auto label = staticLabels.begin(); label != staticLabels.end(); label++)
   {
      g->draw_text(label->position, label->text, label->feature->bounds.width(), label->feature->bounds.height());
   }
}

double getScaleFactor()
{
   return mainCanvas->get_camera().get_world_scale_factor().x;
}

void clickedOnFeature(MapFeature* clicked, ezgl::application* app) //function to manage clicks on a specific MapFeature
{
   mapFeatures.highlightFeature(clicked, true);
   if(clicked->featureType == FEATURE) //clicked on feature
   {
      LatLon currentFeaturePoint = getFeaturePoint(clicked->dataIndex, 0);
      IntersectionIdx closestInter = findClosestIntersection(currentFeaturePoint);

      app->update_message("Clicked POI: " + getFeatureName(clicked->dataIndex) + ", Near: " + getKnownIntersectionName(closestInter));

   }
   if(clicked->featureType == ROAD && clicked->dataIndex < getNumStreetSegments()) //clicked on street segment
   {
      StreetSegmentInfo segmentInfo = getStreetSegmentInfo(clicked->dataIndex);
      string streetName = getStreetName(segmentInfo.streetID);
      app->update_message("Clicked Street: " + streetName);
      
   }
   if(clicked->featureType == MapFeatureType::INTERSECTION && clicked->dataIndex < getNumIntersections()) //clicked on intersection
   {
      string intersection = getKnownIntersectionName(clicked->dataIndex);
      app->update_message("Clicked Intersection: " + intersection);
      prevClickedIntersection = clicked->dataIndex;

   }
   mainCanvas->redraw();
}

void on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y)
{
   if(mapFeatures.layerMap.size() == 0){return;} //if there are no features, skip click detection

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

         if(!feature->major && feature->featureType != MapFeatureType::INTERSECTION){continue;} //only register clicks on major features or intersections
         if(!feature->wasRendered){continue;} //dont register non-rendered/invisible objects as clickable
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

   numberOfStreets = getNumStreets();

   //Cache street lengths
   for(StreetIdx streetIndex = 1; streetIndex < numberOfStreets; streetIndex++)
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
      intersectionFeatures.push_back(mapFeature); //relate feature to corrosponding intersection index
   }

   //cout << "loading segments: " << streetSegmentPoints.size() << endl;
   for(StreetSegmentIdx streetSegmentIndex = 0; streetSegmentIndex < getNumStreetSegments(); streetSegmentIndex++)
   {
      vector<LatLon> curvePoints = *getStreetSegmentPoints(streetSegmentIndex); //get cached curve points for street segment
      vector<ezgl::point2d>* points = new vector<ezgl::point2d>;

      int longestSegmentIndex = 0; //index of point before longest subsegment of street segment  
      double longestLength = 0;
      
      int curvePointCount = curvePoints.size();
      for(int pointIndex = 0; pointIndex < curvePointCount; pointIndex++)
      {
         LatLon curvePoint = curvePoints[pointIndex];
         ezgl::point2d translatedPoint = LatLonToPoint2D(curvePoint);
         points->push_back(translatedPoint);

         if(pointIndex < curvePointCount - 1)
         {
            //calculate distance between points
            double xDistance = translatedPoint.x - LatLonToPoint2D(curvePoints[pointIndex+1]).x;
            double yDistance = translatedPoint.y - LatLonToPoint2D(curvePoints[pointIndex+1]).y;

            double distance = sqrt(xDistance*xDistance + yDistance*yDistance);
            if(distance > longestLength) //check if this segment is the longest so far
            {
               longestLength = distance;
               longestSegmentIndex = pointIndex;
            }
         }
      }
      auto segmentInfo = getStreetSegmentInfo(streetSegmentIndex); //street segment info
   
      ezgl::color roadColor = kColorBasicRoad; //colour of feature (default is building) 

      //if road is oneway, indicate it using segment info
      if(segmentInfo.oneWay)
      {
         roadColor = kColorOnewayRoad;
      }
      
      MapFeature* mapFeature = new MapFeature(1,points,roadColor,LINE,MapFeatureType::ROAD,(int)streetSegmentIndex);  //create map featur object
      mapFeatures.addFeature(mapFeature); //add feature to feature list
      intersectionFeatures.push_back(mapFeature); //relate feature to corresponding intersection index
      //add longest street data
      mapFeature->longestPointSegmentIndex = longestSegmentIndex;
      mapFeature->longestSegmentLength = longestLength;
      //set street id
      StreetIdx streetID = segmentInfo.streetID; //get street ID
      mapFeature->streetID = streetID; //set street id of feature to calculated street id

      //post initialization settings for one way streets
      mapFeature->directional = segmentInfo.oneWay;

      //check if street is a major street
      if(streetID > 0) //street ID 0 can never be a major road as it is the default ID for minor roads
      {  
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

   vector<string> featureNames;
   
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
      double maximumScaling = DBL_MAX;
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
            maximumScaling = kMaximumBuildingScaling; //set maximum distance for buildings to start drawing
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
         case GLACIER:{
            featureColor = kColorBeach; 
            layer = -6;
         }
         default: {
            break;
         }
      }
      
      //default draw type is a polygon
      DrawType drawType = POLYGON;
      //if feature is not closed, then it is a line
      if(!closed){drawType = LINE;}

      

      MapFeature* mapFeature = new MapFeature(layer,points,featureColor,drawType,MapFeatureType::FEATURE,(int)featureIndex); //create map feature object
      mapFeatures.addFeature(mapFeature); //add map feature object to array
      //set default values
      mapFeature->maximumScaling = maximumScaling;
      //add text label for feature name
      string featureName = getFeatureName(featureIndex);
      if(featureName[0] != '<') //dont print text that is not meant to be displayed (denoted by being wrapped in '<>')
      {
         //check for duplicates
         bool duplicate = vectorContains<string>(featureNames, featureName); //whether the name is a duplicate, checked by seeing if name is in list of names already displayed
         if(!duplicate || featureType == BUILDING) //buildings often have the same name, so duplicates can be allowed for those features
         {
            //if no duplicate is found, add label
            staticLabels.push_back({featureName,mapFeature->bounds.center(),mapFeature}); //create static text structure and add to array for rendering
            mapFeature->major = true; //if the feature has a name, it is a point of interest
            featureNames.push_back(featureName); //detect duplicate
         }
      }
   }

   if(enableCellRendering){
      mapRenderCells(kRenderCellSize, minPoints, mapFeatures, cellMap);
   }

   //download weather map data
   currentScope.min = minPoints;
   currentScope.max = maxPoints;
   //Load weather map
   mapTiles = downloadWeatherMaps(NOFILTER, currentScope, 1920, 1080, time(nullptr));
   
   currentWeather = getWeatherData(
      LatLon(
         (currentScope.max.latitude() + currentScope.min.latitude())/2,
         (currentScope.max.longitude() + currentScope.min.longitude())/2
      ),
      currentTime
   );
}

void unloadFeatures()
{
   mapFeatures.clear(); //delete all elements in the map
   cellMap.clear(); //delete all elements in cell map
   staticLabels.clear(); //delete all static labels
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
   //create window
   application.run(initial_setup,on_mouse_click,nullptr,nullptr);

}

void initial_setup(ezgl::application *application, bool /*new_window*/){

   //initializing search button for two streets to intersection search
   GObject* searchBtn = application->get_object("searchBtn");
   g_signal_connect(searchBtn,"clicked",G_CALLBACK(searchBtn_cbk),application);

   //initializing button that opens pop up window with map choices
   GObject* SelectMapBtn = application->get_object("SelectMapBtn");
   g_signal_connect(SelectMapBtn,"clicked",G_CALLBACK(selectMap_cbk),application);

   //initizlizing back button on the pop up window with map choices
   GObject* mapBackBtn = application->get_object("mapBackBtn");
   g_signal_connect(mapBackBtn,"clicked",G_CALLBACK(mapBackBtn_cbk),application);

   //initialize each button on switch map pop-up
   for(int i = 0; i < 19; i++){
      mapNames[i].second.second = application;
      GObject* mapSelectionBtn = application->get_object(mapNames[i].first.c_str());
      g_signal_connect(mapSelectionBtn,"clicked",G_CALLBACK(openMap_cbk),&mapNames[i].second);
   }

   //initialize each filter button
   for(int i = 0; i < 6; i++){
      filterNames[i].second.second = application;
      GObject* filterBtn = application->get_object(filterNames[i].first.c_str());
      g_signal_connect(filterBtn,"clicked",G_CALLBACK(changeFilter_cbk),&filterNames[i].second);
   }
   
   //Display sunrise onto ui
   std::time_t sunriseTimeFull = currentWeather.sunrise;
   string sunriseTime = asctime(std::localtime(&sunriseTimeFull));
   sunriseTime = sunriseTime.substr(11, 5);
   GtkLabel* sunriseLabel = GTK_LABEL(application->get_object("SunriseTimeLabel"));
   gtk_label_set_text(sunriseLabel,sunriseTime.c_str());

   //Display sunset onto ui
   std::time_t sunsetTimeFULL = currentWeather.sunset;
   string sunsetTime = asctime(std::localtime(&sunsetTimeFULL));
   sunsetTime = sunsetTime.substr(11, 5);
   GtkLabel* sunsetLabel = GTK_LABEL(application->get_object("SunsetTimeLabel"));
   gtk_label_set_text(sunsetLabel,sunsetTime.c_str());

   GtkRange* timeSlider = GTK_RANGE(application->get_object("TimeSlider"));
   g_signal_connect(timeSlider,"value-changed",G_CALLBACK(changeCurrentTime_cbk),application);

   //show current time on slider
   time_t selectedTime = std::time(nullptr);

   //change date and time on ui
   string selectedTimeText = asctime(std::localtime(&selectedTime));
   selectedTimeText = selectedTimeText.substr(0,16);
   GtkLabel* selectedTimeLabel = GTK_LABEL(application->get_object("selectedTimeLabel"));
   gtk_label_set_text(selectedTimeLabel,selectedTimeText.c_str());
}

void searchBtn_cbk(GtkWidget */*widget*/, ezgl::application *application){

   //unhighlight previously searched intersections
   for(int i = 0; i < prevSearchedIntersectionIdxs.size(); i++){
      intersectionFeatures[prevSearchedIntersectionIdxs[i]]->highlight = false;
   }
   prevSearchedIntersectionIdxs.clear();

   if(prevClickedIntersection != -1){
      intersectionFeatures[prevClickedIntersection]->highlight = false;
   }
   
   //Extracting text from search bars
   GtkEntry* searchbar1 = GTK_ENTRY(application->get_object("searchbar1"));
   string searchText1 = string(gtk_entry_get_text(searchbar1));
   GtkEntry* searchbar2 = GTK_ENTRY(application->get_object("searchbar2"));
   string searchText2 = string(gtk_entry_get_text(searchbar2));

   //Obtain the streetIdx(s) of the inputted street names
   vector<StreetIdx> street1Idxs = findStreetIdsFromPartialStreetName(searchText1);
   vector<StreetIdx> street2Idxs = findStreetIdsFromPartialStreetName(searchText2);

   //Create unique pairs of streetIdxs of the inputted streets
   set<pair<StreetIdx,StreetIdx>> streetIdPairs;
   for(int i = 0; i < street1Idxs.size(); ++i){
      for(int j = 0; j < street2Idxs.size(); ++j){

         //ensure no pair is made up of the same street
         StreetIdx currStreet1 = street1Idxs[i];
         StreetIdx currStreet2 = street2Idxs[j];

         if(currStreet1 != currStreet2){

            //insert pair by <lower number,higher number> to prevent duplicate sets
            if(currStreet1 < currStreet2){
               streetIdPairs.insert({currStreet1,currStreet2});

            } else{
               streetIdPairs.insert({currStreet2,currStreet1});
            }
         }
      }
   }

   //Store the all intersections of each pair of streets
   vector<vector<IntersectionIdx>> intersectionList;
   auto it = streetIdPairs.begin();
   while(it != streetIdPairs.end()){
      intersectionList.push_back(findIntersectionsOfTwoStreets(*it));
      it++;
   }
   int numOfIntersectionLists = intersectionList.size();
   if(numOfIntersectionLists == 0){
      application->update_message("No intersections found");
      return;
   }

   //Dislay names of intersections in list
   string intersectionInfo = "Searched Intersections: ";

   // Mark each intersection for highlighting
   for(int i = 0; i < numOfIntersectionLists; i++){
      for(int j = 0; j < intersectionList[i].size(); j++){
         mapFeatures.highlightFeature(intersectionFeatures[intersectionList[i][j]], false);

         intersectionInfo.append(getIntersectionName(intersectionList[i][j]) + (", "));

         prevSearchedIntersectionIdxs.push_back(intersectionList[i][j]);
         mapFeatures.highlightFeature(intersectionFeatures[intersectionList[i][j]], true);
      }
   }

   intersectionInfo.pop_back();
   intersectionInfo.pop_back();
   application->update_message(intersectionInfo);
   application->refresh_drawing();
}

void selectMap_cbk(GtkWidget */*widget*/, ezgl::application *application){
   GtkDialog * mapSelectorDialog = GTK_DIALOG(application->get_object("mapSelectorDialog"));
   gtk_dialog_run(mapSelectorDialog);
}

void mapBackBtn_cbk(GtkWidget */*widget*/, ezgl::application *application){
   GtkWidget * mapSelectorDialog = GTK_WIDGET(application->get_object("mapSelectorDialog"));
   gtk_widget_hide(mapSelectorDialog);
}

void resetCanvasParameters(ezgl::application* app) //helper function to reset the canvas if values are changed
{
   //initialHeight = -1;
   //initialWidth = -1;
   initial_world = ezgl::rectangle(LatLonToPoint2D(minPoints),LatLonToPoint2D(maxPoints));
   app->change_canvas_world_coordinates("MainCanvas",initial_world);
   //delete mainCanvas;
   mainCanvas = app->get_canvas("MainCanvas");
   app->refresh_drawing();
}

void openMap_cbk(GtkWidget */*widget*/, pair<string,ezgl::application*> * mapInfo){
   
   string nextMapPath = mapInfo->first;
   ezgl::application* application = mapInfo->second;

   //Close Map Selection menu
   GtkWidget * mapSelectorDialog = GTK_WIDGET(application->get_object("mapSelectorDialog"));
   gtk_widget_hide(mapSelectorDialog);

   //DEALLOCATE THINGS
   prevSearchedIntersectionIdxs.clear();
   
   closeMap();

   bool load_success = loadMap(nextMapPath);
   if(!load_success) {
      std::cerr << "Failed to load map '" << nextMapPath << "'\n";
      return;
   }
   std::cout << "Successfully loaded map '" << nextMapPath << "'\n";

   //Unload old map info
   unloadFeatures();

   //Load new map info
   loadFeatures();


   //Display sunrise onto ui
   std::time_t sunriseTimeFull = currentWeather.sunrise;
   string sunriseTime = asctime(std::localtime(&sunriseTimeFull));
   sunriseTime = sunriseTime.substr(11, 5);
   GtkLabel* sunriseLabel = GTK_LABEL(application->get_object("SunriseTimeLabel"));
   gtk_label_set_text(sunriseLabel,sunriseTime.c_str());

   //Display sunset onto ui
   std::time_t sunsetTimeFULL = currentWeather.sunset;
   string sunsetTime = asctime(std::localtime(&sunsetTimeFULL));
   sunsetTime = sunsetTime.substr(11, 5);
   GtkLabel* sunsetLabel = GTK_LABEL(application->get_object("SunsetTimeLabel"));
   gtk_label_set_text(sunsetLabel,sunsetTime.c_str());

   //Get new paramters
   resetCanvasParameters(application);
}

void changeFilter_cbk(GtkWidget */*widget*/, pair<WeatherLayer,ezgl::application*> * filterInfo){

   //update current weather filter
   mapTiles = downloadWeatherMaps(filterInfo->first, currentScope, 1920, 1080, currentTime);
   ezgl::application* application = filterInfo->second;
   currWeatherLayer = filterInfo->first;

   application->refresh_drawing();
}

void changeCurrentTime_cbk(GtkRange * sliderRange, ezgl::application* application){
   
   //range from 0 days to 10 days in increments of 1/3 days
   double selectedRange = gtk_range_get_value(sliderRange);

   //current time plus number of 1/3 days times epoch time of 1/3 days
   currentTime = std::time(nullptr) + selectedRange*28800*100;

   //change date and time on ui
   string selectedTimeText = asctime(std::localtime(&currentTime));
   selectedTimeText = selectedTimeText.substr(0,16);
   GtkLabel* selectedTimeLabel = GTK_LABEL(application->get_object("selectedTimeLabel"));
   gtk_label_set_text(selectedTimeLabel,selectedTimeText.c_str());

   //refresh value
   mapTiles = downloadWeatherMaps(currWeatherLayer, currentScope, 1920, 1080, currentTime);
   application->refresh_drawing();

   //Display sunrise onto ui
   std::time_t sunriseTimeFull = currentWeather.sunrise;
   string sunriseTime = asctime(std::localtime(&sunriseTimeFull));
   sunriseTime = sunriseTime.substr(11, 5);
   GtkLabel* sunriseLabel = GTK_LABEL(application->get_object("SunriseTimeLabel"));
   gtk_label_set_text(sunriseLabel,sunriseTime.c_str());

   //Display sunset onto ui
   std::time_t sunsetTimeFULL = currentWeather.sunset;
   string sunsetTime = asctime(std::localtime(&sunsetTimeFULL));
   sunsetTime = sunsetTime.substr(11, 5);
   GtkLabel* sunsetLabel = GTK_LABEL(application->get_object("SunsetTimeLabel"));
   gtk_label_set_text(sunsetLabel,sunsetTime.c_str());
}