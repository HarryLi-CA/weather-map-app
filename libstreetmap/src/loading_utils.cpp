#include "loading_utils.h"
#include <vector>
#include <set>
#include "StreetsDatabaseAPI.h"

using namespace std;
void loadIntersections(vector<vector<StreetSegmentIdx>*>& connectedStreetSegments, vector<vector<IntersectionIdx>*>& connectedIntersections, IntersectionIdx& intersectionIndex)
{
    int numberOfSegments = getNumIntersectionStreetSegment(intersectionIndex); //get number of segments in intersection
    
    //allocate street segment vector to be filled
    vector<StreetSegmentIdx>* allocatedStreetVector = new vector<StreetSegmentIdx>();
    connectedStreetSegments[intersectionIndex] = allocatedStreetVector; 

    for(int intersectionSegmentIndex = 0; intersectionSegmentIndex < numberOfSegments; intersectionSegmentIndex++)
    {
        StreetSegmentIdx streetSegmentIndex = getIntersectionStreetSegment(intersectionIndex, intersectionSegmentIndex);
        (connectedStreetSegments)[intersectionIndex]->push_back(streetSegmentIndex);
        cacheStreetSegmentToIntersection(connectedIntersections,intersectionIndex,streetSegmentIndex);
    }
}

void cacheStreetSegmentToIntersection(vector<vector<IntersectionIdx>*>& connectedIntersections, IntersectionIdx intersectionIndex, StreetSegmentIdx streetSegmentIndex)
{
    if(connectedIntersections[streetSegmentIndex] == nullptr) //check if vector for  exists
    {
        //allocate intersection vector to be filled
        vector<IntersectionIdx>* allocatedIntersectionVector = new vector<IntersectionIdx>();
        connectedIntersections[streetSegmentIndex] = allocatedIntersectionVector;
    }
    connectedIntersections[streetSegmentIndex]->push_back(intersectionIndex); //add street segment to intersection
}

void loadStreetSegmentPoint(vector<vector<LatLon>*>& streetSegmentPoints, vector<float>& speedLimitCache, StreetSegmentIdx streetSegmentIndex)
{
    StreetSegmentInfo segmentInfo = getStreetSegmentInfo(streetSegmentIndex);
    vector<LatLon>* pointVector = new vector<LatLon>(); //allocate vector of segment points
    streetSegmentPoints[streetSegmentIndex] = pointVector; //add pointer of vector to loading vector
    speedLimitCache[streetSegmentIndex] = segmentInfo.speedLimit;
    pointVector->push_back(getIntersectionPosition(segmentInfo.from)); //add position of "from" intersection
    //iterate through all intermediate curve points
    for(int curvePointIndex = 0; curvePointIndex < segmentInfo.numCurvePoints; curvePointIndex++)
    {
        pointVector->push_back(getStreetSegmentCurvePoint(streetSegmentIndex,curvePointIndex)); //add curve point to vector
    }
    pointVector->push_back(getIntersectionPosition(segmentInfo.to)); //add position of "to" intersection as final point
}

void loadStreetToStreetSegments(vector<vector<StreetSegmentIdx>*>& streetToStreetSegments){
    int numStreets = getNumStreets();
    for(int i = 0; i < numStreets;i++){
        //cout << "loaded " <<  i <<endl;
        streetToStreetSegments.push_back(new vector<StreetSegmentIdx>());
        //cout << streetToStreetSegments[i]->size() << endl;
    }
    int streetSegments = getNumStreetSegments();
    for(StreetSegmentIdx i = 0; i < streetSegments; i++){ 
        StreetSegmentInfo streetSegInfo = getStreetSegmentInfo(i);
        //cout << streetToStreetSegments[streetSegInfo.streetID]->size() << " "<< streetSegInfo.streetID << endl;
        streetToStreetSegments[streetSegInfo.streetID]->push_back(i);
        //cout << streetToStreetSegments[streetSegInfo.streetID]->size()<<endl;
    }
}

void loadStreetIntersections(vector<vector<IntersectionIdx>*> &streetToIntersections, int numStreetSegments, StreetIdx currStreet)
{
    streetToIntersections[currStreet] = new vector<IntersectionIdx>();

    //store UNIQUE intersectionIdxs of the currStreet into this set, this is used to track whether an intersection has already been inserted into the vector
    set<IntersectionIdx> intersectionTracker;
    int numAddedIntersections = 0;

    //loop through all the street segments in the city, look at the street they are a part of, if the street is the one we want, add their connected intersections into the vector
    for(StreetSegmentIdx streetSegmentIndex = 0; streetSegmentIndex < numStreetSegments; streetSegmentIndex++)
    {
        StreetSegmentInfo segInfo = getStreetSegmentInfo(streetSegmentIndex);
        
        if(segInfo.streetID == currStreet)
        {
            //the intersection is only added to the vector if it was able to be inserted into the list, ie was unique
            intersectionTracker.insert(segInfo.from);
            if(intersectionTracker.size() != numAddedIntersections){
                streetToIntersections[currStreet]->push_back(segInfo.from);
                numAddedIntersections++;
            }

            intersectionTracker.insert(segInfo.to);
            if(intersectionTracker.size() != numAddedIntersections){
                streetToIntersections[currStreet]->push_back(segInfo.to);
                numAddedIntersections++;
            }
        }
    }
}
