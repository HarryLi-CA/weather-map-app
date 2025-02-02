#pragma once
#include <vector>
#include <unordered_map>
#include "StreetsDatabaseAPI.h"
using namespace std;

struct StreetPairHash {
    size_t operator () (const std::pair<StreetIdx, StreetIdx>& streetPair) const {
    auto hash1 = hash<StreetIdx>{}(streetPair.first);
    auto hash2 = hash<StreetIdx>{}(streetPair.second);
    return hash1 ^ hash2;
    }
};

struct StreetPairEqual {
    bool operator () (const std::pair<StreetIdx, StreetIdx>& streetPair1, const std::pair<StreetIdx, StreetIdx>& streetPair2) const {
    return (streetPair1.first == streetPair2.first && streetPair1.second == streetPair2.second) || (streetPair1.first == streetPair2.second && streetPair1.second == streetPair2.first);
    }
};

//Finds connected street segments to a given intersection and store them in a vector 
void loadIntersections(vector<vector<StreetSegmentIdx>*>& connectedStreetSegments, vector<vector<IntersectionIdx>*>& connectedIntersections, IntersectionIdx& intersectionIndex);

//Stores connected street segment in a 2-dimensoinal vector based on connected intersections (will create vector if it does not exist)
void cacheStreetSegmentToIntersection(vector<vector<IntersectionIdx>*>& connectedIntersections, IntersectionIdx intersectionIndex, StreetSegmentIdx streetSegmentIndex);

//Finds all position points of a street segment corrosponding to a given street segment index and loads it into an array
void loadStreetSegmentPoint(vector<vector<LatLon>*>& streetSegmentPoints, vector<float>& speedLimitCache, StreetSegmentIdx streetSegmentIndex);

// Stores all segments part of a street
void loadStreetToStreetSegments(vector<vector<StreetSegmentIdx>*>& streetToStreetSegments);

// Delete all pointers in a vector
template <typename P>
void delete2DVectorPointers(vector<vector<P>*>& toDeleteFrom)
{
    //loop through every pointer in vector
    for(int vectorIndex = 0; vectorIndex < toDeleteFrom.size(); vectorIndex++)
    {
        delete toDeleteFrom[vectorIndex]; //delete vector
    }
    toDeleteFrom.clear(); //remove allocated space in vector
}
void loadStreetSegmentPoint(vector<vector<LatLon>*>& streetSegmentPoints, StreetSegmentIdx streetSegmentIndex);

//Finds intersections asscociated to a street and stores them in a vector
void loadStreetIntersections(vector<vector<IntersectionIdx>*> &streetToIntersections, int numStreetSegments, StreetIdx currStreet);

//Finds pair ascociated with a given intersectionIndex and stores that pair along with its intersection into the data structure
void loadTwoStreetsIntersections(unordered_map<pair<StreetIdx,StreetIdx>,vector<IntersectionIdx>*,StreetPairHash,StreetPairEqual> &twoStreetsToIntersections,vector<StreetSegmentIdx> &streetSegmentsOfIntersection,IntersectionIdx currIntersectionIndex);
