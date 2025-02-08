/*
General Helper Functions
*/
#pragma once
#include <iostream>
#include <vector>
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "Helper.h"

using namespace std;

//helper function to get intersection over a segment from the other intersection
IntersectionIdx findOppositeIntersection(IntersectionIdx& intersection_id, StreetSegmentIdx& connect_street_segment_id, vector<vector<IntersectionIdx>*>& streetSegmentsToIntersections);