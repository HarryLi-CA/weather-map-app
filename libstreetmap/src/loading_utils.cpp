#include "loading_utils.h"
#include <vector>
#include "StreetsDatabaseAPI.h"

using namespace std;
void loadIntersections(vector<vector<StreetSegmentIdx>*>& connectedStreetSegments, IntersectionIdx& intersectionIndex)
{
    int numberOfSegments = getNumIntersectionStreetSegment(intersectionIndex); //get number of segments in intersection
    
    //allocate vector to be added to street segments
    vector<StreetSegmentIdx>* allocatedVector = new vector<StreetSegmentIdx>();
    connectedStreetSegments[intersectionIndex] = allocatedVector; 

    for(int intersectionSegmentIndex = 0; intersectionSegmentIndex < numberOfSegments-1; intersectionSegmentIndex++)
    {
        (connectedStreetSegments)[intersectionIndex]->push_back(getIntersectionStreetSegment(intersectionIndex, intersectionSegmentIndex));
    }
}