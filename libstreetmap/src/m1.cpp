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
 
#include <iostream>
#include "m1.h"
#include "m2.h"
#include "loading_utils.h"
#include "nodeIDHelper.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "Helper.h"
#include "OSMAPIHelper.h"
#include <thread>
#include <vector>
#include <cmath>
#include <cfloat>
#include <limits>
#include <stdexcept>
#include <unordered_map>
#include <map>
#include <iomanip>
#include <set>

using namespace std;
//cache vectors
//takes intersection index and returns vector of indices of all connected street segments
static vector<vector<StreetSegmentIdx>*> intersectionToStreetSegments;
//takes street segment index and returns vector of intersections that connect to the street segment
static  vector<vector<IntersectionIdx>*> streetSegmentsToIntersections;
//takes street index and returns a vector of intersections that connect to the street
static vector<vector<IntersectionIdx>*> streetToIntersections;
//takes street index and returns a set of intersections
static vector<set<IntersectionIdx>*> streetToIntersectionsHelper;
//takes street segment index and returns lat lon positions for all points in sequnce including intersections and curve points
static vector<vector<LatLon>*> streetSegmentPoints;
// stores street segments of streets
static vector<vector<StreetSegmentIdx>*> streetToStreetSegments;
//takes two street indexs as a pair and returns vector of intersections that the two streets intersect at,
//Uses a special hash function StreetPairHash that allows pairs of StreetIdxs to be used as keys
//Uses a special comparator StreetPairEqual that allows pairs to be equated (ie <1,2> == <2,1>)
static unordered_map<pair<StreetIdx,StreetIdx>,vector<IntersectionIdx>*,StreetPairHash,StreetPairEqual> twoStreetsToIntersections;
//contains all street names, with their respective streetIds and can be accessed with just a partial string of the name
//Uses a special comparator stringComparator to allow for street names to be identified by a partial string
static multimap<string,StreetIdx,stringComparator> streetNames;
//stores length of street segment
static vector<double> streetSegmentLengthCache;
static vector<double> streetSpeedLimitCache;
static vector<double> travelTimeCache;

int numIntersections = 0;
int numStreetSegments = 0;
int numStreets = 0;


// loadMap will be called with the name of the file that stores the "layer-2"
// map data accessed through StreetsDatabaseAPI: the street and intersection 
// data that is higher-level than the raw OSM data). 
// This file name will always end in ".streets.bin" and you 
// can call loadStreetsDatabaseBIN with this filename to initialize the
// layer 2 (StreetsDatabase) API.
// If you need data from the lower level, layer 1, API that provides raw OSM
// data (nodes, ways, etc.) you will also need to initialize the layer 1 
// OSMDatabaseAPI by calling loadOSMDatabaseBIN. That function needs the 
// name of the ".osm.bin" file that matches your map -- just change 
// ".streets" to ".osm" in the map_streets_database_filename to get the proper
// name.

bool loadMap(std::string map_streets_database_filename) {
    //load map and check if successful
    bool load_successful = loadStreetsDatabaseBIN(map_streets_database_filename); //Indicates whether the map has loaded 
                                  //successfully
    string osm_file = map_streets_database_filename.substr(0,map_streets_database_filename.size()-11)+ "osm.bin";
    bool load_osm_success = loadOSMDatabaseBIN(osm_file); // load OSM database
    
    std::cout << "loadMap: " << map_streets_database_filename << std::endl;

    //
    // Load your map related data structures here.
    //
    //if load fails, return false
    if (!load_successful || !load_osm_success){
        cout << "load fail" << endl;
        return false;
    }

    osmhelp::load();
    numIntersections = osmhelp::getNumIntersections(); //load number of intersections once to avoid excessive calculations
    numStreetSegments = osmhelp::getNumStreetSegments(); //load number of street segments once to avoid excessive calculations
    numStreets = osmhelp::getNumStreets(); //load number of streets

    intersectionToStreetSegments.resize(numIntersections);
    streetSegmentsToIntersections.resize(numStreetSegments,nullptr);
    streetSegmentPoints.resize(numStreetSegments, nullptr);
    streetSegmentLengthCache.resize(numStreetSegments, -1);
    streetSpeedLimitCache.resize(numStreetSegments, -1);
    travelTimeCache.resize(numStreetSegments, -1);
    streetToIntersections.resize(numStreets);
    streetToIntersectionsHelper.resize(numStreets);

    //load all the segment indices for each intersection index
    for(IntersectionIdx intersectionIndex = 0; intersectionIndex < numIntersections; intersectionIndex++)
    {
        loadIntersections(intersectionToStreetSegments,streetSegmentsToIntersections, intersectionIndex);

        //load the container twoStreetsToIntersections with all street pairs that meet at intersectionIndex
        loadTwoStreetsIntersections(twoStreetsToIntersections,*intersectionToStreetSegments[intersectionIndex],intersectionIndex);
    }
    loadStreetToStreetSegments(streetToStreetSegments);
    for(StreetIdx streetIndex = 0; streetIndex < numStreets; streetIndex++)
    {
        streetNames.insert({getStreetName(streetIndex),streetIndex});
        streetToIntersections[streetIndex] = new vector<IntersectionIdx>();
        streetToIntersectionsHelper[streetIndex] = new set<IntersectionIdx>();
    }
    for(StreetSegmentIdx streetSegmentIndex = 0; streetSegmentIndex < numStreetSegments; streetSegmentIndex++)
    {
        loadStreetSegmentPoint(streetSegmentPoints,streetSpeedLimitCache,streetSegmentIndex);
        loadStreetIntersections(streetToIntersections,streetToIntersectionsHelper,streetSegmentIndex);
    }
    //wait for all intersection threads
    cout << "done!" << endl;

    load_successful = true; //Make sure this is updated to reflect whether
                            //loading the map succeeded or failed

    return load_successful;
}

void closeMap() {
    unloadGraphics();
    //Clean-up your map related data structures here
    //delete and clear all elements in vectors with dynamically allocated data
    delete2DVectorPointers<StreetSegmentIdx>(intersectionToStreetSegments);
    delete2DVectorPointers<IntersectionIdx>(streetSegmentsToIntersections);
    delete2DVectorPointers<IntersectionIdx>(streetToIntersections);
    delete2DVectorPointers<LatLon>(streetSegmentPoints);
    delete2DVectorPointers<IntersectionIdx>(streetToStreetSegments);
    osmhelp::reset();

    auto twoStreetIntersectionIt = twoStreetsToIntersections.begin();
    while(twoStreetIntersectionIt != twoStreetsToIntersections.end()){
        delete twoStreetIntersectionIt->second;
        twoStreetIntersectionIt->second = nullptr;
        twoStreetIntersectionIt++;
    }

    for(StreetIdx streetIndex = 0; streetIndex < numStreets; streetIndex++)
    {
        delete streetToIntersectionsHelper[streetIndex];
        streetToIntersectionsHelper[streetIndex] = nullptr;
    }

    twoStreetsToIntersections.clear();

    streetNames.clear();

    clearIdHelper(); // marks OSMID hashtable for reset
    closeStreetDatabase();
    closeOSMDatabase();
    //clear map
    
    unloadFeatures();
    clearHeapPointers();
}

double findDistanceBetweenTwoPoints(LatLon point_1, LatLon point_2){
    double x1,x2,y1,y2; //cartesian points based on Earth's surface LatLon points
    double latavg = (point_1.latitude()+ point_2.latitude())/2; //average latitude

    double lon1 = point_1.longitude();
    double lon2 = point_2.longitude();
    
    if(abs(lon1-lon2) > 180) // check if the case is across -180 and 180 longitude
        lon1 += (lon1<0)?360:-360;

    //Calculate equivalent cartesian points using equirectangular projection
    x1 = kEarthRadiusInMeters * lon1*kDegreeToRadian * cos(latavg*kDegreeToRadian);
    x2 = kEarthRadiusInMeters * lon2*kDegreeToRadian * cos(latavg*kDegreeToRadian);
    y1 = kEarthRadiusInMeters * point_1.latitude()*kDegreeToRadian;
    y2 = kEarthRadiusInMeters * point_2.latitude()*kDegreeToRadian;
    double xDif = x1-x2; //horizontal distance between point 1 and point 2 
    double yDif = y1-y2; //vertical distance between point 1 and point 2
    return sqrt(xDif*xDif+yDif*yDif); //use pythagorean theorem to calculate final distance, return result
}

double findStreetSegmentLength(StreetSegmentIdx street_segment_id){
    //check if street segment id is valid (between 0 and the maximum index)
    if(street_segment_id < 0 || street_segment_id >= numStreetSegments)
    {
        return -1; //return -1 to indicate street segment id is invalid
    }
    //check if length is cached
    if(streetSegmentLengthCache[street_segment_id] != -1)
    {
        return streetSegmentLengthCache[street_segment_id]; //if segment length is cached, return cached value
    }
    StreetSegmentInfo streetSegInfo = osmhelp::getStreetSegmentInfo(street_segment_id); //get street segment info
    double streetSegmentLength = 0; //initialize total segment length to 0
    //iterate through all curve points
    LatLon fromCurvePoint = osmhelp::getIntersectionPosition(streetSegInfo.from); //get first curve point
    LatLon toCurvePoint; //initialize sequential curve point
    vector<LatLon> curvePoints = *streetSegmentPoints[street_segment_id];
    for(int curvePointIndex = 0; curvePointIndex < streetSegmentPoints[street_segment_id]->size() - 1; curvePointIndex++)
    {
        toCurvePoint = curvePoints[curvePointIndex + 1]; //get next curve point
        streetSegmentLength += findDistanceBetweenTwoPoints(fromCurvePoint, toCurvePoint); //add the curve segment length to the total
        
        fromCurvePoint = toCurvePoint; //set the next from point to the current to point, since the to point will be updated on the next iteration
    }
    //add street segment to global cache
    streetSegmentLengthCache[street_segment_id] = streetSegmentLength;
    return streetSegmentLength; //return result
}


//helper function to cache travel time in order for fast retrieval in findStreetSegmentTravelTime


double findStreetSegmentTravelTime(StreetSegmentIdx street_segment_id){
    //check if street segment id is valid (between 0 and the maximum index)
    if(street_segment_id < 0 || street_segment_id >= numStreetSegments)
    {
        return -1; //return -1 to indicate street segment id is invalid
    }
    //if travel time is not cached, calculate it
    if(travelTimeCache[street_segment_id] == -1)
    {
        double travelSpeed = streetSpeedLimitCache[street_segment_id]; //get maximum speed on road segment (m/s)
        double travelDistance = findStreetSegmentLength(street_segment_id);  //get total length of segment
        double travelTime = travelDistance/travelSpeed; //calculate travel time using time elapsed = distance/speed

        travelTimeCache[street_segment_id] = travelTime; //return result
    }
    return travelTimeCache[street_segment_id]; //return result from cache
}

double findStreetSegmentTurnAngle(StreetSegmentIdx src_street_segment_id,StreetSegmentIdx dst_street_segment_id){
    //check if src or dst street segment id is valid (between 0 and the maximum index)
    bool srcSegmentInBounds = src_street_segment_id >= 0 && src_street_segment_id < numStreetSegments; //boolean that checks if src street segment index is inside the bounds of street segment vector 
    bool dstSegmentInBounds = dst_street_segment_id >= 0 && dst_street_segment_id < numStreetSegments; //boolean that checks if dst street segment index is inside the bounds of street segment vector
    //check if either the src or dst index is invalid
    if(!srcSegmentInBounds || !dstSegmentInBounds)
    {

        return NO_ANGLE; //return identifier for no results to indicate one of the street segment ids is invalid
    }
    //get street segment info
    StreetSegmentInfo srcInfo = osmhelp::getStreetSegmentInfo(src_street_segment_id); //src street info
    StreetSegmentInfo dstInfo = osmhelp::getStreetSegmentInfo(dst_street_segment_id); //dst street info
    //intialize common values
    IntersectionIdx intersectionID; //index of common intersection
    LatLon intersectionPoint; //latlon position of common intersetcion
    //check if segments do not share intersection
    //calculate exit points of src/dst street
    vector<LatLon> srcCurvePoints = *streetSegmentPoints[src_street_segment_id]; //list of latlon points along src segment
    vector<LatLon> dstCurvePoints = *streetSegmentPoints[dst_street_segment_id]; //list of latlon points along dst segment
    if(srcInfo.to == dstInfo.from || srcInfo.to == dstInfo.to) //check if common instersection is src's to and dst's from
    {
        intersectionID = srcInfo.to;
    }else if(srcInfo.from == dstInfo.from  || srcInfo.from == dstInfo.to) //check if common instersection is src's from and dst's from
    {
        intersectionID = srcInfo.from;   
    }
    else
    {
        return NO_ANGLE; //if segments do not share any common intersections, return an identifier for no results
    }
    intersectionPoint = osmhelp::getIntersectionPosition(intersectionID); //get position of intersection
    
    LatLon srcExitPoint; //point right before intersection along src segment
    LatLon dstEnterPoint; //point right after intersection along dst segment
    //check which intersections are shared in src and dst
    if(intersectionID == srcInfo.from) //src check
    {
        srcExitPoint = srcCurvePoints[1]; //get point right after "from" intersection
    }
    else
    {
        srcExitPoint = srcCurvePoints[srcCurvePoints.size() - 2]; //get point right before "to" intersection
    }
    
    if(intersectionID == dstInfo.from) //dst check
    {
        dstEnterPoint = dstCurvePoints[1]; //get point right after "from" intersection
    }
    else
    {
        dstEnterPoint = dstCurvePoints[dstCurvePoints.size() - 2]; //get point right before "to" intersection
    } 
    
    double srcToDst = findDistanceBetweenTwoPoints(srcExitPoint,dstEnterPoint); //distance between src exit point and dst enter point
    double srcToIntersection = findDistanceBetweenTwoPoints(srcExitPoint,intersectionPoint); //distance between src exit point and intersection
    double dstToIntersection = findDistanceBetweenTwoPoints(dstEnterPoint,intersectionPoint); //distance between dst enter point and intersection

    //cosine law to calculate cos of turn angle
    double cosAngle = (srcToDst*srcToDst - srcToIntersection*srcToIntersection - dstToIntersection*dstToIntersection)/(-2*dstToIntersection*srcToIntersection);
    double angle = abs(acos(cosAngle)); //turn angle between segments     
    return M_PI - angle;
}


//helper function to get intersection over a segment from the other intersection
IntersectionIdx findOppositeIntersection(IntersectionIdx intersection_id, StreetSegmentIdx connect_street_segment_id)
{
    
    //check if the intersection is the first intersection in the connected intersection array for the street segment
    if(intersection_id == (*streetSegmentsToIntersections[connect_street_segment_id])[0])
    {
        return (*streetSegmentsToIntersections[connect_street_segment_id])[1]; //if true, then return the other intersection
    }
    else
    {
        return (*streetSegmentsToIntersections[connect_street_segment_id])[0]; //if false, then return the intersection that was just checked
    }
}

std::vector<IntersectionIdx> findAdjacentIntersections(IntersectionIdx intersection_id)
{
    //check if intersection id is valid (between 0 and the maximum index)
    if(intersection_id < 0 || intersection_id >= numIntersections)
    {
        return {}; //return -1 to indicate intersection id is invalid
    }
    vector<IntersectionIdx> adjacentIntersections; //dynamically allocated vector of adjacent intersections
    vector<StreetSegmentIdx>* connectedStreetSegments = intersectionToStreetSegments[intersection_id]; //vector containing all connected streets to the intersection
    //iterate through all streets
    for(int connectedStreetSegmentVectorIndex = 0; connectedStreetSegmentVectorIndex < connectedStreetSegments->size(); connectedStreetSegmentVectorIndex++)
    {
        StreetSegmentIdx streetSegmentIndex = (*connectedStreetSegments)[connectedStreetSegmentVectorIndex]; //connected street segment index   
        StreetSegmentInfo segInfo = osmhelp::getStreetSegmentInfo(streetSegmentIndex); //info of connected street segment
        
        //if the segment is a one-way road and the intersection is not the roading being driven out of, skip this iteration
        if(segInfo.oneWay && segInfo.from != intersection_id)
        {
            continue;
        }
        
        IntersectionIdx otherIntersection = findOppositeIntersection(intersection_id,streetSegmentIndex, streetSegmentsToIntersections); //other intersection on connected street segment

        

        bool skipPush = false; //skip flag for whether to add to the vector or not
        //check for duplicates in allocated vector
        for(int adjacentIntersectionIndex = 0; adjacentIntersectionIndex < adjacentIntersections.size(); adjacentIntersectionIndex++)
        {
            //if duplicate is found, set flag to skip iteration in loop
            if((adjacentIntersections)[adjacentIntersectionIndex] == otherIntersection)
            {
                skipPush = true;
                break;
            }
        }

        //if skip flag is true, skip over this iteration of the loop
        if(skipPush)
        {
            continue;
        }

        adjacentIntersections.push_back(otherIntersection); //add intersection to allocated vector
    }
    return adjacentIntersections; //return allocated vector
}

IntersectionIdx findClosestIntersection(LatLon my_position){
    int closestNode = -1; // return -1 if no intersections exist
    double closestDist = DBL_MAX; // initial value as maximum distance
    for(int i = 0; i < numIntersections; i++){ // loop through intersections
        LatLon curLatLon = osmhelp::getIntersectionPosition(i);
        double curDist = findDistanceBetweenTwoPoints(my_position, curLatLon); // find distance of current interection with point
        if(i==0 || closestDist > curDist){ // save minimum distance
            closestDist = curDist;
            closestNode = i;
        }
    }
    return closestNode;
}

std::vector<StreetSegmentIdx> findStreetSegmentsOfIntersection(IntersectionIdx intersection_id){
    if(intersection_id >= 0 && intersection_id < numIntersections){
        return *intersectionToStreetSegments[intersection_id];

    } else {
        return {};
    }
}

std::vector<StreetSegmentIdx>* findStreetSegmentsOfIntersectionPtr(IntersectionIdx intersection_id){
    if(intersection_id >= 0 && intersection_id < numIntersections){
        return intersectionToStreetSegments[intersection_id];

    } else {
        return {};
    }
}

std::vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id){
    if(street_id >= 0 && street_id < numStreets){
        return *streetToIntersections[street_id];

    } else {
        return {};
    }
}

//Searches the unordered_map twoStreetsToIntersections for a key with the same pair as street_ids
//Pairs will work in any order ie <1,2> = <2,1>, if pair is not found an empty vector is returned
std::vector<IntersectionIdx> findIntersectionsOfTwoStreets(std::pair<StreetIdx, StreetIdx> street_ids){

    bool exists = twoStreetsToIntersections.find(street_ids) != twoStreetsToIntersections.end();

    if(exists){
        return *twoStreetsToIntersections[street_ids];

    } else {
        return {};
    }
}

//Obtains all elements in the multimap streetNames that start with the street_prefix
std::vector<StreetIdx> findStreetIdsFromPartialStreetName(std::string street_prefix){
    
    vector<StreetIdx> newVector = {};

    //An empty string as input will return a vector with element -1
    if(street_prefix.length() == 0){
        return newVector;
    }

    //Grabs a pair of iterators, 
    //The first pointing to the first key with the given street_prefix
    //The second pointing to the key after the last key with the prefix
    //the multimap is in sorted order, therefore all street names with the prefix will be back to back
    auto matchRange = streetNames.equal_range(street_prefix);
    for(auto low = matchRange.first; low != matchRange.second; low++){
        StreetIdx streetIdxInRange = low->second;
        newVector.push_back(streetIdxInRange);
    }
    
    return newVector;
}

double findStreetLength(StreetIdx street_id){
    double out = 0;
    vector<StreetSegmentIdx> streetSegs = *(streetToStreetSegments[street_id]); // get street segments
    int streetSegments = streetSegs.size(); // get amount of segments to loop through
    for(int i = 0; i < streetSegments; i++){
        out+= findStreetSegmentLength(streetSegs[i]); // add street segment lengths
    };
    return out;
}

LatLonBounds findStreetBoundingBox (StreetIdx street_id){
    LatLonBounds out; // output of function
    int streetSegments = getNumStreetSegments();
    vector<LatLon> locations = {};
    
    for(StreetSegmentIdx i = 0; i < streetSegments; i++){ // loop through each street segment
        StreetSegmentInfo streetSegInfo = osmhelp::getStreetSegmentInfo(i);
        if(streetSegInfo.streetID == street_id){ //check if current segment is part of the street
            int curvesAmt = streetSegInfo.numCurvePoints;
            locations.push_back(osmhelp::getIntersectionPosition(streetSegInfo.from)); // push all points to vector
            locations.push_back(osmhelp::getIntersectionPosition(streetSegInfo.to));
            for(int j = 0; j < curvesAmt; j++){
                locations.push_back(getStreetSegmentCurvePoint(i,j));
            }
        }
    };
    
    //error check if street has street segments
    if(locations.size() == 0)
        return out;
    double minLat, minLon, maxLat, maxLon;
    minLat = locations[0].latitude(); // go through each location and find the max and min values
    maxLat = locations[0].latitude();
    minLon = locations[0].longitude();
    maxLon = locations[0].longitude();
    for(int i = 1; i < locations.size(); i++){
        double curLat, curLon;
        curLat = locations[i].latitude();
        curLon = locations[i].longitude();
        minLat = min(minLat, curLat);
        maxLat = max(maxLat, curLat);
        minLon = min(minLon, curLon);
        maxLon = max(maxLon, curLon);
    }

    out.min = LatLon(minLat, minLon);
    out.max = LatLon(maxLat, maxLon);
    return out;
}

POIIdx findClosestPOI(LatLon my_position, std::string poi_type){
    POIIdx closestNode = 0;
    double closestDist = DBL_MAX; // initial value as maximum distance
    int numOfPOI = getNumPointsOfInterest();
    for(POIIdx i = 0; i < numOfPOI; i++){ // loop through POIs
        if(getPOIType(i) == poi_type){
            LatLon curLatLon = getPOIPosition(i);
            double curDist = findDistanceBetweenTwoPoints(my_position, curLatLon); // find distance of current interection with point
            if(i==0 || closestDist > curDist){ // save minimum distance
                closestDist = curDist;
                closestNode = i;
            }
        }
    }
    return closestNode;
}

double findFeatureArea(FeatureIdx feature_id)
{
    //check is the feature_id is valid by checking if it is between 0 and the number of features
    if(feature_id < 0 || feature_id > getNumFeatures())
    {
        return -1; //return -1 to indicate feature_id is invalid 
    }
    int numPoints = getNumFeaturePoints(feature_id); //number of vertices in feature
    //check if shape is polygon by checking if first point is equal to last point
    bool isPolygon = getFeaturePoint(feature_id, 0) == getFeaturePoint(feature_id, numPoints - 1); //whether the feature is a closed polygon or not

    //if feature is not a closed polygon, return 0
    if(!isPolygon)
    {
        return 0;
    }


    //find boundbox (minimum latitude and longitude in feature)
    double latBound = DBL_MAX; //initialize bounds to max
    double lonBound = DBL_MAX; //initialize bounds to max
    for(int pointIndex = 0; pointIndex < numPoints; pointIndex++)
    {
        LatLon featurePoint = getFeaturePoint(feature_id, pointIndex); //current point in feature
        //check for minimum latitude
        if(featurePoint.latitude() < latBound)
        {
            latBound = featurePoint.latitude();
        }
        //check for minimum longitude
        if(featurePoint.longitude() < lonBound)
        {
            lonBound = featurePoint.longitude();
        }
    }
    
    double totalArea = 0; //total area of the feature
    LatLon currentFeaturePoint = getFeaturePoint(feature_id, 0); 
    for(int pointIndex = 1; pointIndex < numPoints; pointIndex++)
    {
        LatLon nextFeaturePoint = getFeaturePoint(feature_id, pointIndex); //current point in feature

        //get points on the y/x axis to calculate the vertical and horizontal distances from the relative axis bounds
        LatLon relativeNextLonAxisPoint = LatLon(nextFeaturePoint.latitude(), lonBound); //point on the longitude axis that shares a latitude with nextFeaturePoint
        LatLon relativeNextLatAxisPoint = LatLon(latBound, nextFeaturePoint.longitude()); //point on the latitude axis that shares a longitude with nextFeaturePoint
        LatLon relativeCurrentLonAxisPoint = LatLon(currentFeaturePoint.latitude(), lonBound); //point on the longitude axis that shares a latitude with currentFeaturePoint
        LatLon relativeCurrentLatAxisPoint = LatLon(latBound, currentFeaturePoint.longitude()); //point on the latitude axis that shares a longitude with currentFeaturePoint

        //find values required to calculate area of trapezoid
        double y2 = findDistanceBetweenTwoPoints(nextFeaturePoint, relativeNextLonAxisPoint);
        double x2 = findDistanceBetweenTwoPoints(nextFeaturePoint, relativeNextLatAxisPoint);
        
        double y1 = findDistanceBetweenTwoPoints(currentFeaturePoint, relativeCurrentLonAxisPoint);
        double x1 = findDistanceBetweenTwoPoints(currentFeaturePoint, relativeCurrentLatAxisPoint);

        double area = (y2 - y1)*(x2 + x1)/2; //area of trapezoid created by the current point, the next point and the latitude/longitude bounds

        //add to total area of shape
        totalArea += area;

        //set current feature to next feature
        currentFeaturePoint = nextFeaturePoint;
    }
    //to account for differences the starting height of the first point (which may cause negative values), return the absolute value of the area
    return abs(totalArea);
}

double findWayLength(OSMID way_id){
    const OSMWay* osmWay = getWayByID(way_id); // use helper function to speed up receiving the way
    if(osmWay == nullptr)
        return 0;
    vector<OSMID> wayNodes = getWayMembers(osmWay);
    const OSMNode* prevNode = getNodeByID(wayNodes[0]); // use helper function to speed up receiving each node
    double length = 0;
    for(int i = 1; i < wayNodes.size(); i++){ // loop through all ways to find the distance between them
        const OSMNode* curNode = getNodeByID(wayNodes[i]);
        length += findDistanceBetweenTwoPoints(prevNode->coords(), curNode->coords());
        prevNode = curNode;
    }
    return length;
}

std::string getOSMNodeTagValue(OSMID osm_id, std::string key){
    const OSMNode* node = getNodeByID(osm_id); //use helper function to speed up finding correct node
    if(node == nullptr) // error check if node with ID doesn't exist
        return "";
    int tagAmt = getTagCount(node);
    for(int i = 0; i < tagAmt; i++){ // loop until correct key found
        string osmKey, value;
        tie(osmKey, value) = getTagPair(node, i);
        if(osmKey == key)
            return value;
    }
    return "";
}

std::vector<LatLon>* getStreetSegmentPoints(StreetSegmentIdx streetSegmentIndex)
{
    return streetSegmentPoints[streetSegmentIndex]; //get pointer for curve point vector of street segments and return it
}

std::vector<StreetSegmentIdx>* getIntersectionStreetSegments(IntersectionIdx intersectionIndex)
{
    return intersectionToStreetSegments[intersectionIndex]; //get pointer for curve point vector of street segments and return it
}
