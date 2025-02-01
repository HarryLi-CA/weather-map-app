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
#include "loading_utils.h"
#include "nodeIDHelper.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include <thread>
#include <vector>
#include <cmath>
#include <cfloat>
#include <limits>
#include <unordered_map>

using namespace std;
//cache vectors

//takes intersection index and returns vector of indices of all connected street segments
vector<vector<StreetSegmentIdx>*> intersectionToStreetSegments;
//takes street segment index and returns vector of intersections that connect to the street segment
vector<vector<IntersectionIdx>*> streetSegmentsToIntersections;
//takes street segment index and returns lat lon positions for all points in sequnce including intersections and curve points
vector<vector<LatLon>*> streetSegmentPoints;
int numIntersections = 0;
int numStreetSegments = 0;
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
    
    std::cout << "loadMap: " << map_streets_database_filename << std::endl;

    //
    // Load your map related data structures here.
    //
    //if load fails, return false

    numIntersections = getNumIntersections(); //load number of intersections once to avoid excessive calculations
    numStreetSegments = getNumStreetSegments(); //load number of street segments once to avoid excessive calculations
    
    intersectionToStreetSegments.resize(numIntersections);
    streetSegmentsToIntersections.resize(numStreetSegments,nullptr);
    streetSegmentPoints.resize(numStreetSegments, nullptr);

    //load all the segment indices for each intersection index
    for(IntersectionIdx intersectionIndex = 0; intersectionIndex < numIntersections; intersectionIndex++)
    {
        loadIntersections(intersectionToStreetSegments,streetSegmentsToIntersections, intersectionIndex);
        //cout << intersection_index+1 << "/" << num_intersections << endl;
    }
    for(StreetSegmentIdx streetSegmentIndex = 0; streetSegmentIndex < numStreetSegments; streetSegmentIndex++)
    {
        loadStreetSegmentPoint(streetSegmentPoints,streetSegmentIndex);
    }
    //wait for all intersection threads
    cout << "done!";

    load_successful = true; //Make sure this is updated to reflect whether
                            //loading the map succeeded or failed

    return load_successful;
}

void closeMap() {
    //Clean-up your map related data structures here
    closeStreetDatabase();
}

double findDistanceBetweenTwoPoints(LatLon point_1, LatLon point_2){
    double x1, x2,y1,y2, xDif, yDif, latavg;
    double crosssectionRadius = 
    latavg = (point_1.latitude()+ point_2.latitude())/2;
    x1 = kEarthRadiusInMeters * point_1.longitude()*kDegreeToRadian * cos(latavg*kDegreeToRadian);
    x2 = kEarthRadiusInMeters * point_2.longitude()*kDegreeToRadian * cos(latavg*kDegreeToRadian);
    y1 = kEarthRadiusInMeters * point_1.latitude()*kDegreeToRadian;
    y2 = kEarthRadiusInMeters * point_2.latitude()*kDegreeToRadian;
    xDif = x1-x2;
    yDif = y1-y2;
    return sqrt(xDif*xDif+yDif*yDif);
}

double findStreetSegmentLength(StreetSegmentIdx street_segment_id){
    StreetSegmentInfo streetSegInfo = getStreetSegmentInfo(street_segment_id); //get street segment info
    double streetSegmentLength = 0; //initialize total segment length to 0
    //iterate through all curve points
    LatLon fromCurvePoint = getIntersectionPosition(streetSegInfo.from); //get first curve point
    LatLon toCurvePoint; //initialize sequential curve point
    vector<LatLon> curvePoints = *streetSegmentPoints[street_segment_id];
    for(int curvePointIndex = 0; curvePointIndex < streetSegmentPoints[street_segment_id]->size() - 1; curvePointIndex++)
    {
        toCurvePoint = curvePoints[curvePointIndex + 1]; //get next curve point
        streetSegmentLength += findDistanceBetweenTwoPoints(fromCurvePoint, toCurvePoint); //add the curve segment length to the total
        
        fromCurvePoint = toCurvePoint; //set the next from point to the current to point, since the to point will be updated on the next iteration
    }
    return streetSegmentLength; //return result
}

double findStreetSegmentTravelTime(StreetSegmentIdx street_segment_id){
    
    
    double travelSpeed = getStreetSegmentInfo(street_segment_id).speedLimit; //get maximum speed on road segment (m/s)
    double travelDistance = findStreetSegmentLength(street_segment_id);  //get total length of segment
    double travelTime = travelDistance/travelSpeed; //calculate travel time using time elapsed = distance/speed

    return travelTime; //return result
}

double findStreetSegmentTurnAngle(StreetSegmentIdx src_street_segment_id,StreetSegmentIdx dst_street_segment_id){
    //get street segment info
    StreetSegmentInfo srcInfo = getStreetSegmentInfo(src_street_segment_id); //src street info
    StreetSegmentInfo dstInfo = getStreetSegmentInfo(dst_street_segment_id); //dst street info
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
        return NO_ANGLE;
    }
    intersectionPoint = getIntersectionPosition(intersectionID); //get position of intersection
    
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
    
    //cout << "src exit point:"<<srcExitPoint<<endl;
    double srcToDst = findDistanceBetweenTwoPoints(srcExitPoint,dstEnterPoint);
    //cout << findDistanceBetweenTwoPoints(dstCurvePoints[1],intersectionPoint) << "|" <<  findDistanceBetweenTwoPoints(srcCurvePoints[srcCurvePoints.size() - 2],intersectionPoint) << "|" << findDistanceBetweenTwoPoints(srcExitPoint,dstExitPoint) << endl;
    double srcToIntersection = findDistanceBetweenTwoPoints(srcExitPoint,intersectionPoint); //distance between src exit point and intersection
    double dstToIntersection = findDistanceBetweenTwoPoints(dstEnterPoint,intersectionPoint); //distance between dst enter point and intersection

    //cosine law to calculate cos of turn angle
    double cosAngle = (srcToDst*srcToDst - srcToIntersection*srcToIntersection - dstToIntersection*dstToIntersection)/(-2*dstToIntersection*srcToIntersection);
    double angle = abs(acos(cosAngle)); //turn angle between segments     
    return M_PI - angle;
}

std::vector<IntersectionIdx> findAdjacentIntersections(IntersectionIdx intersection_id){return {};}

IntersectionIdx findClosestIntersection(LatLon my_position){
    int closestNode = 0;
    double closestDist = DBL_MAX; // initial value as maximum distance
    for(int i = 0; i < numIntersections; i++){ // loop through intersections
        //OSMNode* curNode = getNodeByIndex(i);
        LatLon curLatLon = getNodeByIndex(i)->coords();
        double curDist = findDistanceBetweenTwoPoints(my_position, curLatLon); // find distance of current interection with point
        if(i==0 || closestDist > curDist){ // save minimum distance
            closestDist = curDist;
            closestNode = i;
        }
    }
    return closestNode;
}

std::vector<StreetSegmentIdx> findStreetSegmentsOfIntersection(IntersectionIdx intersection_id){return {};}

std::vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id){return {};}

std::vector<IntersectionIdx> findIntersectionsOfTwoStreets(std::pair<StreetIdx, StreetIdx> street_ids){return {};}

std::vector<StreetIdx> findStreetIdsFromPartialStreetName(std::string street_prefix){return {};}

double findStreetLength(StreetIdx street_id){return 0;}

LatLonBounds findStreetBoundingBox (StreetIdx street_id){
    //loadNodeValueByID(nodeIDReference);
    const OSMWay* osmWay = getWayByIndex(street_id);
    const vector<OSMID> nodes = osmWay->ndrefs();
    LatLonBounds out;
    double minLat, minLon, maxLat, maxLon;
    const OSMNode* curNode = getNodeByID(nodes[0]);
    minLat = curNode->coords().latitude();
    maxLat = curNode->coords().latitude();
    minLon = curNode->coords().longitude();
    maxLon = curNode->coords().longitude();

    for(int i = 1; i < nodes.size(); i++){
        curNode = getNodeByID(nodes[i]);
        double curLat, curLon;
        curLat = curNode->coords().latitude();
        curLon = curNode->coords().longitude();
        minLat = min(minLat, curLat);
        maxLat = max(minLat, curLat);
        minLon = min(minLon, curLon);
        maxLon = max(minLon, curLon);
    }
    LatLon* minLatLon = new LatLon(minLat, minLon);
    LatLon* maxLatLon = new LatLon(maxLat, maxLon);

    out.min = *minLatLon;
    out.max = *maxLatLon;
    return out;
}

POIIdx findClosestPOI(LatLon my_position, std::string poi_type){return 0;}

double findFeatureArea(FeatureIdx feature_id){return 0;}

double findWayLength(OSMID way_id){return 0;}

std::string getOSMNodeTagValue(OSMID osm_id, std::string key){return NULL;}
