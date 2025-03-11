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
    double opacity = 0.5;
    string palette = "";
    LatLonBounds tileBounds;
    string url;
    bool isLoaded = false;
    WeatherLayer wL = NOFILTER;
};

//sunset sunrise in epoch timestamps
struct WeatherData {
    int sunrise;
    int sunset;
};

struct WeatherPalette{
    int value = 0;
    string color = "";
};

// Get global coordiantes of mercator projection from latitude and longitude
pair<double,double> getGlobalCoordsFromLatLon(LatLon l, double mapWidth);

// Get latlon from global coordinates of mercantor projection
LatLon getLatLonFromGlobalCoords(pair<double,double> coords, double mapWidth);

//Get map tiles based on current screen
vector<MapTileData> getMapTilesFromBounds (LatLonBounds currentScope, int pixelLength, int pixelHeight, WeatherLayer wL);

//converts enum WeatherLayer to api string
string convertWeatherLayerToString(WeatherLayer wL);

//Converts tile and weather to specific api url
string getMapAPIURLFromParameters(WeatherLayer wL, MapTileData tileData, int date, double opacity = 0.5, string pallete = "");

// download weather maps for current city
void downloadWeatherMaps(WeatherLayer wL, vector<MapTileData>& mapTiles, int date);

// helps download imgs via threads
void downloadImgFromURL(string outputURL, string imgUrl);

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