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

    //DID NOT CHANGE ANY CODE, COMMENTED OUT CODE WAS FOR MY TESTING

    int numberOfSegments = getNumIntersectionStreetSegment(intersectionIndex); //get number of segments in intersection
    
    //allocate street segment vector to be filled
    vector<StreetSegmentIdx>* allocatedStreetVector = new vector<StreetSegmentIdx>();
    connectedStreetSegments[intersectionIndex] = allocatedStreetVector; 
    // cout << "load Intersection " << intersectionIndex << ": " << endl;
    // cout << "Steets: ";
    for(int intersectionSegmentIndex = 0; intersectionSegmentIndex < numberOfSegments; intersectionSegmentIndex++)
    {
        StreetSegmentIdx streetSegmentIndex = getIntersectionStreetSegment(intersectionIndex, intersectionSegmentIndex);
        (connectedStreetSegments)[intersectionIndex]->push_back(streetSegmentIndex);
        // cout << streetSegmentIndex << "(" << getStreetSegmentInfo(streetSegmentIndex).streetID << ")" << ", ";
        cacheStreetSegmentToIntersection(connectedIntersections,intersectionIndex,streetSegmentIndex);
    }
    // cout << endl;
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
            if(intersectionTracker.size() != numAddedIntersections)
            {
                streetToIntersections[currStreet]->push_back(segInfo.from);
                numAddedIntersections++;
            }

            intersectionTracker.insert(segInfo.to);
            if(intersectionTracker.size() != numAddedIntersections)
            {
                streetToIntersections[currStreet]->push_back(segInfo.to);
                numAddedIntersections++;
            }
        }
    }
}

//This fucntion takes in a given intersectionIndex(currIntersectionIndex) and adds all streets that pass through that given intersection in pairs to the container twoStreetsToIntersections.
//All streets that touch the given intersection are passed in from streetSegmentsOfCurrIntersection as street segments.
//The container twoStreetsToIntersections maps a pair of streets (they key) to a vector of intersections (the value) that involve those streets.
//There will be no duplicate pairs or pairs that are made up of two of the same street (ie <1,1>).
void loadTwoStreetsIntersections(unordered_map<pair<StreetIdx,StreetIdx>,vector<IntersectionIdx>*,StreetPairHash,StreetPairEqual> &twoStreetsToIntersections,vector<StreetSegmentIdx> &streetSegmentsOfCurrIntersection,IntersectionIdx currIntersectionIndex)
{
    //IGNORE ALL COMMENTED OUT CODE, JUST FOR TESTING
    // cout << "load TWO Intersection " << currIntersectionIndex << ": " << endl;
    // cout << "Steets array: ";
    int numOfStreetSegs = streetSegmentsOfCurrIntersection.size();

    // for(int i = 0; i < numOfStreetSegs; i++){
    //     cout << getStreetSegmentInfo(streetSegmentsOfCurrIntersection[i]).streetID << " , ";
    // }

    // cout << endl << "imnputting: ";

    set<pair<StreetIdx,StreetIdx>> pairSet;

    //loop through all the street segments at the current intersection, store unique pairs of street ids into a set
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
        // cout << i->first << " , " << i->second << endl;
        if(pairExists)
        {
            // cout << "already exists" << endl;
            twoStreetsToIntersections[*i]->push_back(currIntersectionIndex);
        }
        else
        {
            // cout << "creating new" << endl;
            vector<IntersectionIdx>* newVec = new vector<IntersectionIdx>();
            newVec->push_back(currIntersectionIndex);
            twoStreetsToIntersections.insert(make_pair(*i,newVec));
        }
        i++;
    }
    
    // cout << endl;
}