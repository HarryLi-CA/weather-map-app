#include <limits.h>
#include <vector>
#include <cmath>
#include <curl/curl.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <thread>
#include <utility>
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

    //double mapWidth    = mapWidth;
    double mapHeight   = mapWidth;

    double x = (longitude+180)*(mapWidth/360); // x val

    double mercN = log(tan((PI/4)+(latitude*kDegreeToRadian/2)));
    double y     = (mapHeight/2)-(mapWidth*mercN/(2*PI)); // get y value without offset
    return make_pair(x, y);
}

LatLon getLatLonFromGlobalCoords(pair<double,double> coords, double mapWidth){
    double x    = coords.first; // (φ)
    double y   = coords.second;   // (λ)

    //double mapWidth    = mapWidth;
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



vector<MapTileData> getMapTilesFromBounds (LatLonBounds currentScope, int pixelLength, int pixelHeight, WeatherLayer wL){
    if(wL == NOFILTER)
        return{};
    int optimalTilesWide = pixelLength / TILEWIDTH + 1; // minimum tiles for max resolution 
    int optimalTilesHigh = pixelHeight / TILEWIDTH + 1;
    
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

    // rearrange for new coord system
    double temp = minGlobalCoords.second;
    minGlobalCoords.second = maxGlobalCoords.second;
    maxGlobalCoords.second = temp;
    // cout << minGlobalCoords.first << " " << minGlobalCoords.second << endl;
    // cout << maxGlobalCoords.first << " " << maxGlobalCoords.second << endl;

    vector<MapTileData> out;
    vector<double> weightedOpacities = {0.4,0,0,0,0.8,0.8};
    vector<string> palletes = 
    {
        "0:00000000;0.023150:ffffffff",
        "",
        "",
        "",
        "80:00000000;100:ffffffff",
        "0:00000000;200:ffffffff",
    };

    for(int i = (int)minGlobalCoords.first; i <= (int)maxGlobalCoords.first; i++){ 
        for(int j = (int)minGlobalCoords.second; j <= (int)maxGlobalCoords.second; j++){
            LatLonBounds tileBounds;
                tileBounds.min = getLatLonFromGlobalCoords(make_pair(i,j),zoom);
                tileBounds.max = getLatLonFromGlobalCoords(make_pair(i+1,j+1),zoom);
            if(wL == WEIGHTED){
                for(int k = 0; k < weightedOpacities.size(); k++){
                    if(weightedOpacities[k] > 0){
                        MapTileData m; 
                        m.x = i; m.y = j; m.z = z; m.tileBounds = tileBounds; m.wL = (WeatherLayer)(k+1);
                        m.opacity = weightedOpacities[k]; m.palette = palletes[k];
                        m.url = "libstreetmap/imgs/" + to_string(m.z) + "_" + to_string(m.x) + "_" + to_string(m.y);
                        m.url += "_" + to_string(m.wL);
                        m.url += ".png";
                        out.push_back(m);
                    }
                }
            }
            else{
                MapTileData m; 
                m.x = i; m.y = j; m.z = z; m.tileBounds = tileBounds; m.wL = wL;
                m.url = "libstreetmap/imgs/" + to_string(m.z) + "_" + to_string(m.x) + "_" + to_string(m.y);
                m.url += "_" + to_string(wL);
                m.url += ".png";
                out.push_back(m);
            }
        }
    }
    return out;
}

string convertWeatherLayerToString(WeatherLayer wL){
    switch(wL){
        case NOFILTER:
            return "";
            break;
        case PRECIPITATION:
            return "PR0";
            break;
        case SNOW:
            return "SD0";
            break;
        case WIND:
            return "WND";
            break;
        case TEMPERATURE:
            return "TA2";
            break;
        case HUMIDITY:
            return "HRD0";
            break;
        case CLOUDS:
            return "CL";
            break;
        default:
            cout<< "Error weather layer not found: " <<  to_string(wL) << endl;
            throw wL;
            return "";
            break;
    }
}

string getMapAPIURLFromParameters(WeatherLayer wL, MapTileData tileData, int date, double opacity, string pallete){
    string baseURL = "http://maps.openweathermap.org/maps/2.0/weather/";
    baseURL += convertWeatherLayerToString(wL);
    baseURL += '/' + to_string(tileData.z) + '/' + to_string(tileData.x) + '/' + to_string(tileData.y);
    baseURL += "?opacity=" + to_string(opacity);
    baseURL += "&date=" + to_string(date);

    if(pallete.size() > 0)
        baseURL += "&palette="+ pallete;
    // if(pallete.size() >= 2){
    //     baseURL += "&palette=";
    //     for (int i = 0; i < pallete.size(); i++)
    //     {
    //         if(i>0)
    //             baseURL+= ';';
    //         baseURL+= to_string(pallete[i].value) + ":" + pallete[i].color;
    //     }
    // }

    baseURL += "&appid=" + APIKey;
    return baseURL;
}

size_t data_write(void* buf, size_t size, size_t nmemb, void* userp)
{
	if (userp)
	{
		std::ostream& os = *static_cast<std::ostream*>(userp);
		std::streamsize len = size * nmemb;
		if (os.write(static_cast<char*>(buf), len))
			return len;
	}

	return 0;
}


CURLcode curl_read(const std::string& url, std::ostream& os, long timeout = 30)
{
	CURLcode code(CURLE_FAILED_INIT);
	CURL* curl = curl_easy_init();

	if (curl)
	{
		if (CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &data_write))
			&& CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L))
			&& CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L))
			&& CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_FILE, &os))
			&& CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout))
			&& CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_URL, url.c_str())))
		{
			code = curl_easy_perform(curl);
		}
		curl_easy_cleanup(curl);
	}
    curl_global_cleanup();
	return code;
}
CURLcode res;

void downloadImgFromURL(string outputURL, string imgUrl){
    std::ofstream ofs(outputURL, std::ostream::binary);

    if (CURLE_OK == curl_read(imgUrl, ofs))
    {
    }
    ofs.close();
}



void downloadWeatherMaps(WeatherLayer wL, vector<MapTileData>& mapTiles, int date){
    if (wL == NOFILTER)
        return;

    res = curl_global_init(CURL_GLOBAL_ALL);

    vector<pair<MapTileData,string>> urls;
    vector<thread> downloadingThreads;


    for(int i = 0; i < mapTiles.size(); i++){
        MapTileData curMapTile = mapTiles[i];
        string url = getMapAPIURLFromParameters(curMapTile.wL, curMapTile,date, curMapTile.opacity, curMapTile.palette);
        if(url == ""){
            return;
        }
        cout<<url<<endl;
        downloadingThreads.push_back(thread(downloadImgFromURL, curMapTile.url, url));
    }
    for(int i = 0; i < mapTiles.size(); i++){
        downloadingThreads[i].join();
        mapTiles[i].isLoaded = true;
    }
}

string weatherJson = "";

size_t write_weather_data(void *buffer, size_t nmemb) {
    if (buffer && nmemb) {
        weatherJson += (char *)buffer;
    }
    return nmemb;
}

void fetchWeatherData(LatLon location){
    res = curl_global_init(CURL_GLOBAL_ALL);
    weatherJson = "";
    cout << "load1" <<endl;
    if (res != CURLE_OK) {
    cout << "ERROR: Unable to initialize libcurl" << endl;
    cout << curl_easy_strerror(res) << endl;
    return ;
    }
    CURL *curlHandle = curl_easy_init();
    if ( !curlHandle ) {
    cout << "ERROR: Unable to get easy handle" << endl;
    return;
    } else {
    // Set web URL to fetch page/data from
    string apiURL = "https://api.openweathermap.org/data/2.5/forecast/daily?lat="+
        to_string(location.latitude())+
        "&lon=" +
        to_string(location.longitude())+
        "&cnt=16&appid=" + APIKey;
    res = curl_easy_setopt(curlHandle, CURLOPT_URL, apiURL.c_str());
    if (res != CURLE_OK) {
    cout << "ERROR: Unable to set libcurl option" << endl;
    cout << curl_easy_strerror(res) << endl;
    } else {
    // Perform web transfer request
    curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, write_weather_data);
    //curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, weatherJson); 
    res = curl_easy_perform(curlHandle);
    }
        cout<< "at url: " << apiURL <<endl;
    if (res == CURLE_OK) {
    } else if(res == 23){
        res = CURLE_OK;
    } 
    else {
        cout << weatherJson << endl;
        cout<< "at url: " << apiURL <<endl;
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
    while (curIdx != string::npos && time > curDay)
    {
        lastIdx = curIdx;
        curIdx = weatherJson.find("\"dt\":", lastIdx+1);
        if(curIdx != string::npos)
            curDay = stoi(weatherJson.substr(curIdx+5));
    }
    size_t sunriseIdx = weatherJson.find("\"sunrise\":", lastIdx+1);
    size_t sunsetIdx = weatherJson.find("\"sunset\":", lastIdx+1);
    WeatherData out;
    out.sunrise = stoi(weatherJson.substr(sunriseIdx+10));
    out.sunset = stoi(weatherJson.substr(sunsetIdx+9));
    return out;
}