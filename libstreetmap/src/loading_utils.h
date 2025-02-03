#pragma once
#include <vector>
#include "StreetsDatabaseAPI.h"
using namespace std;


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