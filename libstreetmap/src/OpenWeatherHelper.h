#pragma once
#include <limits.h>
#include <vector>
#include <cmath>
#include <curl/curl.h>
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"

#define PI 3.14159265358979323846264338327950288
#define TILEWIDTH 256
#define MAXZ 9
using namespace std;

//data to construct a map tile
struct MapTileData {
    int x;
    int y;
    int z;
    LatLonBounds tileBounds;
};

//sunset sunrise in epoch timestamps
struct WeatherData {
    int sunrise;
    int sunset;
};

// Get global coordiantes of mercator projection from latitude and longitude
pair<double,double> getGlobalCoordsFromLatLon(LatLon l, double mapWidth);

//Get map tiles based on current screen
vector<MapTileData> getMapTilesFromBounds (LatLonBounds currentScope, int pixelHeight, int pixelLength);

//Converts tile and weather to specific api url
string getMapAPIURLFromParameters(WeatherLayer wL, MapTileData tileData);

//helper for libcurl receiving weather data
static size_t write_weather_data(void *buffer, size_t size, size_t nmemb, void *userp);

// fetch weather data with libcurl
void fetchWeatherData(LatLon location);

// fetch data and parse json to weather data
WeatherData getWeatherData(LatLon location, int time);