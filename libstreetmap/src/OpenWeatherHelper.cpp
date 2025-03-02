#include <limits.h>
#include <vector>
#include <cmath>
#include <curl/curl.h>
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "OpenWeatherHelper.h"

#define PI 3.14159265358979323846264338327950288
#define TILEWIDTH 256
#define MAXZ 9
using namespace std;

string APIKey = "***REMOVED***";

pair<double,double> getGlobalCoordsFromLatLon(LatLon l, double mapWidth){
    double latitude    = l.latitude();
    double longitude   = l.longitude();

    double mapWidth    = mapWidth;
    double mapHeight   = mapWidth;

    double x = (longitude+180)*(mapWidth/360); // x val

    double mercN = log(tan((PI/4)+(latitude*kDegreeToRadian/2)));
    double y     = (mapHeight/2)-(mapWidth*mercN/(2*PI)); // get y value without offset
    return make_pair(x, y);
}

LatLon getGlobalCoordsFromLatLon(pair<double,double> coords, double mapWidth){
    double x    = coords.first; // (φ)
    double y   = coords.second;   // (λ)

    double mapWidth    = mapWidth;
    double mapHeight   = mapWidth;

    // x = (longitude+180)*(mapWidth/360)
    // longitude = x/mapwidth*360 - 180
    double longitude = x/mapWidth*360 - 180;
    
    // y     = (mapHeight/2)-(mapWidth*mercN/(2*PI))
    // (-y +(mapHeight/2))/mapWidth * 2 * PI    = (mercN)
    double mercN = (-y +(mapHeight/2))/mapWidth * 2 * PI;

    // mercN = log(tan((PI/4)+(latitude*kDegreeToRadian/2)))
    // (atan(exp(mercN)) - PI/4)/kDegreeToRadian*2= (latitude)
    double lattitude = (atan(exp(mercN)) - PI/4)/kDegreeToRadian*2;
    return LatLon(lattitude, longitude);
}



vector<MapTileData> getMapTilesFromBounds (LatLonBounds currentScope, int pixelHeight, int pixelLength){
    int optimalTilesWide = pixelHeight / TILEWIDTH + 1; // minimum tiles for max resolution 
    int optimalTilesHigh = pixelLength / TILEWIDTH + 1;
    
    int z = 0;
    int zoom = 1;

    pair<double,double> minGlobalCoords = getGlobalCoordsFromLatLon(currentScope.min, zoom);
    pair<double,double> maxGlobalCoords = getGlobalCoordsFromLatLon(currentScope.max, zoom);

    double currentTilesWide = maxGlobalCoords.first - minGlobalCoords.first;
    double currentTilesHigh = maxGlobalCoords.second - minGlobalCoords.second;
    for(;z < MAXZ || (currentTilesWide > optimalTilesWide && optimalTilesHigh>currentTilesHigh); z++){
        currentTilesWide*=2;
        currentTilesHigh*=2;
        zoom*=2;
    }

    //now get real coords
    minGlobalCoords = getGlobalCoordsFromLatLon(currentScope.min, zoom);
    maxGlobalCoords = getGlobalCoordsFromLatLon(currentScope.max, zoom);

    vector<MapTileData> out;
    for(int i = minGlobalCoords.first; i < maxGlobalCoords.first; i++){ 
        for(int j = minGlobalCoords.first; j < maxGlobalCoords.second; j++){
            LatLonBounds tileBounds = {
                .min = getGlobalCoordsFromLatLon(make_pair(i,j),zoom),
                .max = getGlobalCoordsFromLatLon(make_pair(i+1,j+1),zoom)
            };
            MapTileData m = {.x = i, .y = j, .z = z, .tileBounds = tileBounds};
            out.push_back(m);
        }
    }
    return out;
}

enum WeatherLayer{
    PRECIPITATION, //Precipitation intensity
    SNOW, // Depth of snow
    WIND, // Joint display of speed wind (color) and wind direction (arrows)
    TEMPERATURE, //Air temperature at a height of 2 meters
    HUMIDITY, //Relative humidity
    CLOUDS //Cloudiness
};

string getMapAPIURLFromParameters(WeatherLayer wL, MapTileData tileData){
    string baseURL = "http://maps.openweathermap.org/maps/2.0/weather/TS0/3/2/2";
    switch(wL){
        case PRECIPITATION:
            baseURL+= "PR0";
            break;
        case SNOW:
            baseURL+= "SD0";
            break;
        case WIND:
            baseURL+= "SD0";
            break;
        case TEMPERATURE:
            baseURL+= "WND";
            break;
        case HUMIDITY:
            baseURL+= "HRD0";
            break;
        case CLOUDS:
            baseURL+= "CL";
            break;
        default:
            throw wL;
            break;
    }
    baseURL += '/' + tileData.z + '/' + tileData.x + '/' + tileData.y;
    baseURL += "?opacity=0.3&";
    baseURL += "?appid=" + APIKey;
    return baseURL;
}

string weatherJson = "";

static size_t write_weather_data(void *buffer, size_t size, size_t nmemb, void *userp) {
    cout << "In my own custom callback function" << endl;
    if (buffer && nmemb) {
        /* NOTE: This example allocates an extra byte for NULL-terminating the
        * buffer as we know the received data is ASCII text. This may
        * not be always true (e.g. binary data like images).
        */
        char myBuf[nmemb + 1] = {0}; // Zeroes out the array
        strncpy(myBuf, (char *)buffer, nmemb);
        weatherJson = myBuf;
        }
        return nmemb;
}

void fetchWeatherData(LatLon location){
    CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
    if (res != CURLE_OK) {
    cout << "ERROR: Unable to initialize libcurl" << endl;
    cout << curl_easy_strerror(res) << endl;
    return;
    }
    CURL *curlHandle = curl_easy_init();
    if ( !curlHandle ) {
    cout << "ERROR: Unable to get easy handle" << endl;
    return;
    } else {
    // Set web URL to fetch page/data from
    string apiURL = "https://api.openweathermap.org/data/2.5/forecast/daily?lat=" +
    to_string(location.latitude()) +
    "&lon="+to_string(location.longitude()) +
    "&appid=***REMOVED***";

    res = curl_easy_setopt(curlHandle, CURLOPT_URL, apiURL);
    if (res != CURLE_OK) {
    cout << "ERROR: Unable to set libcurl option" << endl;
    cout << curl_easy_strerror(res) << endl;
    } else {
    // Perform web transfer request
    curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, write_weather_data);
    res = curl_easy_perform(curlHandle);
    }
    cout << endl << endl;
    if (res == CURLE_OK) {
    } else {
    cout << "ERROR: res == " << res << endl;
    cout << curl_easy_strerror(res) << endl;
    cout << "See https://curl.haxx.se/libcurl/c/libcurl-errors.html "
    "for error codes" << endl;
    }
    curl_easy_cleanup(curlHandle);
    curlHandle = nullptr;
    }
    curl_global_cleanup();
}

WeatherData getWeatherData(LatLon location, int time){
    fetchWeatherData(location);
    size_t lastIdx = weatherJson.find("\"dt\":");
    size_t curIdx = weatherJson.find("\"dt\":", lastIdx+1);
    int curDay = stoi(weatherJson.substr(curIdx+5));
    while (curIdx != string::npos && time < curDay)
    {
        lastIdx = curIdx;
        curIdx = weatherJson.find("\"dt\":", lastIdx+5);
        if(curIdx != string::npos)
            curDay = stoi(weatherJson.substr(curIdx));
    }
    size_t sunriseIdx = weatherJson.find("\"sunrise\":", lastIdx+1);
    size_t sunsetIdx = weatherJson.find("\"sunset\":", lastIdx+1);
    WeatherData out = {
        .sunrise = stoi(weatherJson.substr(sunriseIdx+10)),
        .sunset = stoi(weatherJson.substr(sunriseIdx+9))
    };
    return out;
}



