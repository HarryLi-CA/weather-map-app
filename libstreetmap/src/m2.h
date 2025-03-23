#pragma once

#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "ezgl/camera.hpp"
#include "ezgl/canvas.hpp"
#include "OpenWeatherHelper.h"
#include "renderHelper.h"
#include "OSMAPIHelper.h"
using namespace std;

void drawMap();
void loadFeatures(); //create feature objects to be drawn on the map
void loadGraphics(ezgl::renderer* g); //load images to be drawn on the map
void unloadFeatures(); //helper function to unload all dynamically allocated data
void unloadGraphics(ezgl::renderer* g); //unload all loaded images
void unloadGraphics(); //unload all loaded images using the last set application
void resetCanvasParameters(ezgl::application* app); //helper function to reset the canvas if values are changed
double getScaleFactor(); //returns scale factor from mainCanvas object

void drawFeature(MapFeature *feature, ezgl::renderer *g, double lineWidth, double scaling, double realWorldScaling, ezgl::camera camera); //helper function to draw specific features on app canvas
void clickedOnFeature(MapFeature* clicked, ezgl::application* app); //helper function to manage clicks on a specific MapFeature

void initial_setup(ezgl::application *application, bool /*new_window*/);

void thread_safe_draw_main_canvas(ezgl::renderer *g); // thread safe draw main canvas only one running at a timevoid threadedChangeFilter_cbk(pair<WeatherLayer,ezgl::application*> filterInfo)
void draw_main_canvas(ezgl::renderer *g); //callback to draw on main canvas
void on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y); //callback when mouse clicks on map
void searchBtn_cbk(GtkWidget */*widget*/, ezgl::application *application); //Callback that is called when search button is pressed
void selectMap_cbk(GtkWidget */*widget*/, ezgl::application *application); //Callback that is called when select new map button is pressed
void mapBackBtn_cbk(GtkWidget */*widget*/, ezgl::application *application); //Callback that is called when pressing the back button in the pop up map selector dialog, closes dialog
void openMap_cbk(GtkWidget */*widget*/, pair<string,ezgl::application*> * mapInfo); //Callback function that changes the map, called when map is selected from the pop up dialog

void threadedChangeFilter_cbk(pair<WeatherLayer,ezgl::application*> filterInfo); // allocate ui action to seperate thread
void changeFilter_cbk(GtkWidget */*widget*/, pair<WeatherLayer,ezgl::application*> *filterInfo); //callback function that happens when any filter button is pressed

void threadedChangeCurrentTime_cbk(GtkRange * sliderRange, ezgl::application* application);
void changeCurrentTime_cbk(GtkRange * sliderRange, ezgl::application* application);
