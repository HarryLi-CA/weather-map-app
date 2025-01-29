#include "loading_utils.h"
#include <vector>
#include "StreetsDatabaseAPI.h"

using namespace std;

void loadIntersections(vector<vector<StreetSegmentIdx>>& connectedStreetSegments, IntersectionIdx& intersectionIndex)
{
    int numberOfSegments = getNumIntersectionStreetSegment(intersectionIndex); //get number of segments in intersection
    connectedStreetSegments.push_back(vector<StreetSegmentIdx>()); //create new vector to 
    for(int intersectionSegmentIndex = 0; intersectionSegmentIndex < numberOfSegments; intersectionSegmentIndex++)
    {
        (connectedStreetSegments)[intersectionIndex].push_back(getIntersectionStreetSegment(intersectionIndex, intersectionSegmentIndex));
    }
}