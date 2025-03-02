#include "loading_utils.h"
#include <vector>
#include <set>
#include <unordered_map>
#include <utility>
#include <iostream>
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
        streetToStreetSegments.push_back(new vector<StreetSegmentIdx>());
    }
    int streetSegments = getNumStreetSegments();
    for(StreetSegmentIdx i = 0; i < streetSegments; i++){ 
        StreetSegmentInfo streetSegInfo = getStreetSegmentInfo(i);
        streetToStreetSegments[streetSegInfo.streetID]->push_back(i);
    }
}

//This function takes in a given streetSegmentIdx and inserts unique intersectionIdxs associated with that given segment into the streetToIntersections container.
//The intersections will be inserted to the vector at index currStreetSeg in streetToIntersections.
//A helper container streetToIntersectionsHelper that mirrors streetToIntersections is used to identify if an intersection is already contained inside the vector.
void loadStreetIntersections(vector<vector<IntersectionIdx>*> &streetToIntersections, vector<set<IntersectionIdx>*> &streetToIntersectionsHelper, StreetSegmentIdx currStreetSeg){
    
    StreetSegmentInfo currSegInfo = getStreetSegmentInfo(currStreetSeg);
    StreetIdx currStreet = currSegInfo.streetID;

    set<IntersectionIdx>* currStreetIntersectionSet = streetToIntersectionsHelper[currStreet];

    int numOfCurrIntersections = currStreetIntersectionSet->size();
    
    //if an intersection was successfully inserted into the set, it was unique
    currStreetIntersectionSet->insert(currSegInfo.from);
    if(numOfCurrIntersections != currStreetIntersectionSet->size()){
        streetToIntersections[currStreet]->push_back(currSegInfo.from);
        numOfCurrIntersections++;
    }

    currStreetIntersectionSet->insert(currSegInfo.to);
    if(numOfCurrIntersections != currStreetIntersectionSet->size()){
        streetToIntersections[currStreet]->push_back(currSegInfo.to);
    }
}

//This fucntion takes in a given intersectionIndex(currIntersectionIndex) and adds all streets that pass through that given intersection in pairs to the container twoStreetsToIntersections.
//All streets that touch the given intersection are passed in from streetSegmentsOfCurrIntersection as street segments.
//The container twoStreetsToIntersections maps a pair of streets (they key) to a vector of intersections (the value) that involve those streets.
//There will be no duplicate pairs or pairs that are made up of two of the same street (ie <1,1>).
void loadTwoStreetsIntersections(unordered_map<pair<StreetIdx,StreetIdx>,vector<IntersectionIdx>*,StreetPairHash,StreetPairEqual> &twoStreetsToIntersections,vector<StreetSegmentIdx> &streetSegmentsOfCurrIntersection,IntersectionIdx currIntersectionIndex){

    int numOfStreetSegs = streetSegmentsOfCurrIntersection.size();

    set<pair<StreetIdx,StreetIdx>> pairSet;

    //loop through all the street segments at the current intersection, store unique pairs of street ids into a set
    //Despite being a double for-loop, the number of streetSegs at a given intersection is at worst about 4 elements
    for(int streetSegIt1 = 0; streetSegIt1 < numOfStreetSegs; ++streetSegIt1)
    {
        for(int streetSegIt2 = streetSegIt1 + 1; streetSegIt2 < numOfStreetSegs; ++streetSegIt2)
        {
            StreetIdx streetAIdx = getStreetSegmentInfo(streetSegmentsOfCurrIntersection[streetSegIt1]).streetID;
            StreetIdx streetBIdx = getStreetSegmentInfo(streetSegmentsOfCurrIntersection[streetSegIt2]).streetID;

            //ensure no pair is made up of the same street
            if(streetAIdx != streetBIdx)
            {
                //insert pair by <lower number,higher number> to prevent duplicate sets
                if(streetAIdx < streetBIdx)
                {
                    pairSet.insert({streetAIdx,streetBIdx});
                }
                else
                {
                    pairSet.insert({streetBIdx,streetAIdx});
                }
            }
            
        }
    }

    //use an iterator to loop through all pairs in the set, place these pairs into the container twoStreetsToIntersections along with their respective intersection currIntersectionIndex
    auto i = pairSet.begin();
    while(i != pairSet.end())
    {
        //if there is a vector of intersections already associated with the particular pair, add it to that list, other wise need to assign that pair a vector
        bool pairExists = twoStreetsToIntersections.find(*i) != twoStreetsToIntersections.end();
        if(pairExists)
        {
            twoStreetsToIntersections[*i]->push_back(currIntersectionIndex);
        }
        else
        {
            vector<IntersectionIdx>* newVec = new vector<IntersectionIdx>();
            newVec->push_back(currIntersectionIndex);
            twoStreetsToIntersections.insert(make_pair(*i,newVec));
        }
        i++;
    }
}