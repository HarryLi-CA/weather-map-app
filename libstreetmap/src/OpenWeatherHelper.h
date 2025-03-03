#pragma once
#include <limits.h>
#include <vector>
#include <cmath>
#include <curl/curl.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"

#define PI 3.14159265358979323846264338327950288
#define TILEWIDTH 256
#define MAXZ 9
using namespace std;

enum WeatherLayer{
    NOFILTER,
    PRECIPITATION, //Precipitation intensity
    SNOW, // Depth of snow
    WIND, // Joint display of speed wind (color) and wind direction (arrows)
    TEMPERATURE, //Air temperature at a height of 2 meters
    HUMIDITY, //Relative humidity
    CLOUDS, //Cloudiness
    WEIGHTED // in progress
};
//data to construct a map tile
struct MapTileData {
    int x;
    int y;
    int z;
    LatLonBounds tileBounds;
    string url;
};

//sunset sunrise in epoch timestamps
struct WeatherData {
    int sunrise;
    int sunset;
};

// Get global coordiantes of mercator projection from latitude and longitude
pair<double,double> getGlobalCoordsFromLatLon(LatLon l, double mapWidth);

// Get latlon from global coordinates of mercantor projection
LatLon getLatLonFromGlobalCoords(pair<double,double> coords, double mapWidth);

//Get map tiles based on current screen
vector<MapTileData> getMapTilesFromBounds (LatLonBounds currentScope, int pixelHeight, int pixelLength);

//Converts tile and weather to specific api url
string getMapAPIURLFromParameters(WeatherLayer wL, MapTileData tileData);

// download weather maps for current city
vector<MapTileData> downloadWeatherMaps(WeatherLayer wL, LatLonBounds currentScope, int pixelHeight, int pixelLength);

//helper for libcurl receiving weather data
size_t write_weather_data(void *buffer, size_t nmemb);

// fetch weather data with libcurl
void fetchWeatherData(LatLon location);

// fetch data and parse json to weather data
WeatherData getWeatherData(LatLon location, int time);

//libcurl read image
CURLcode curl_read(const std::string& url, std::ostream& os, long timeout);
//write data to file stream
size_t data_write(void* buf, size_t size, size_t nmemb, void* userp);