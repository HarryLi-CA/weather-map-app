#pragma once

#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "ezgl/camera.hpp"
#include "OpenWeatherHelper.h"
using namespace std;

void drawMap();
void loadFeatures(); //create feature objects to be drawn on the map
void unloadFeatures(); //helper function to unload all dynamically allocated data
double getScaleFactor(); //returns scale factor from mainCanvas object

void initial_setup(ezgl::application *application, bool /*new_window*/);

void searchBtn_cbk(GtkWidget */*widget*/, ezgl::application *application);
void searchbarAuto1(GtkEntry* searchbar1, ezgl::application *application);
void selectMap_cbk(GtkWidget */*widget*/, ezgl::application *application);
void mapBackBtn_cbk(GtkWidget */*widget*/, ezgl::application *application);
void openMap_cbk(GtkWidget */*widget*/, pair<string,ezgl::application*> * mapInfo);
void changeFilter_cbk(GtkWidget */*widget*/, WeatherLayer *filterInfo);
