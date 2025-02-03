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

// Stores all travel times for each s
void cacheTravelTime(vector<float>& segmentTravelTimeCache, vector<float>& streetSpeedLimitCache, vector<float>& streetSegmentLengthCache,  StreetSegmentIdx street_segment_id);