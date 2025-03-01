#pragma once
#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include "StreetsDatabaseAPI.h"
#include "containerStructs.h"
using namespace std;
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
static vector<float> streetSegmentLengthCache;
static vector<float> streetSpeedLimitCache;
static vector<float> travelTimeCache;