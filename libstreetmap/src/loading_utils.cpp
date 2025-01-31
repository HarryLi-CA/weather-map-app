#include "loading_utils.h"
#include <vector>
#include "StreetsDatabaseAPI.h"

using namespace std;
void loadIntersections(vector<vector<StreetSegmentIdx>*>& connectedStreetSegments, vector<vector<StreetSegmentIdx>*>& connectedIntersections, IntersectionIdx& intersectionIndex)
{
    int numberOfSegments = getNumIntersectionStreetSegment(intersectionIndex); //get number of segments in intersection
    
    //allocate vector to be filled
    vector<StreetSegmentIdx>* allocated_street_vector = new vector<StreetSegmentIdx>();
    //allocate intersection vector to be filled
    vector<StreetSegmentIdx>* allocated_intersection_vector = new vector<StreetSegmentIdx>();
    connectedStreetSegments[intersectionIndex] = allocated_street_vector; 

    for(int intersectionSegmentIndex = 0; intersectionSegmentIndex < numberOfSegments-1; intersectionSegmentIndex++)
    {
        (connectedStreetSegments)[intersectionIndex]->push_back(getIntersectionStreetSegment(intersectionIndex, intersectionSegmentIndex));
    }
}