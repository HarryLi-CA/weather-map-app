#pragma once
void drawMap();
void loadFeatures(); //create feature objects to be drawn on the map
void unloadFeatures(); //helper function to unload all dynamically allocated data
double getScaleFactor(); //returns scale factor from mainCanvas object