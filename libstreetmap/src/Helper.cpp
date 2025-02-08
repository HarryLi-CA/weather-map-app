
#include <iostream>
#include <vector>
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "Helper.h"

using namespace std;

//helper function to get intersection over a segment from the other intersection
IntersectionIdx findOppositeIntersection(IntersectionIdx& intersection_id, StreetSegmentIdx& connect_street_segment_id, vector<vector<IntersectionIdx>*>& streetSegmentsToIntersections)
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