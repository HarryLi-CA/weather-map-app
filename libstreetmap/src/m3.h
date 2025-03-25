/*
* Copyright 2025 University of Toronto
ECE297 Software Design and Communication Winter 2025
* Permission is hereby granted, to use this software and associated
* documentation files (the "Software") in course work at the University
* of Toronto, or for personal use. Other uses are prohibited, in
* particular the distribution of the Software either publicly or to third
* parties.
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#pragma once
#include "ezgl/graphics.hpp"
#include "ezgl/canvas.hpp"
#include "ezgl/application.hpp"
#include "ezgl/color.hpp"
#include "m1.h"
#include "m3.h"
#include "renderHelper.h"
#include "pointMath.h"
#include "StreetsDatabaseAPI.h"
#include "PathFinding.h"
#include "OSMAPIHelper.h"
#include <stdlib.h>
#include <vector>
#include <random>
// Returns the time required to travel along the path specified, in seconds.
// The path is given as a vector of street segment ids, and this function can
// assume the vector either forms a legal path or has size == 0. The travel
// time is the sum of the length/speed-limit of each street segment, plus the
// given turn_penalty (in seconds) per turn implied by the path. If there is
// no turn, then there is no penalty. Note that whenever the street id changes
// (e.g. going from Bloor Street West to Bloor Street East) we have a turn.
double computePathTravelTime(const double turn_penalty, const std::vector<StreetSegmentIdx>& path);


// Returns a path (route) between the start intersection (intersect_id.first)
// and the destination intersection (intersect_id.second), if one exists.
// This routine should return the shortest path
// between the given intersections, where the time penalty to turn right or
// left is given by turn_penalty (in seconds). If no path exists, this routine
// returns an empty (size == 0) vector. If more than one path exists, the path
// with the shortest travel time is returned. The path is returned as a vector
// of street segment ids; traversing these street segments, in the returned
// order, would take one from the start to the destination intersection.
std::vector<StreetSegmentIdx> findPathBetweenIntersections(const double turn_penalty, const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids);

struct PathSegment
{
    IntersectionIdx fromIdx;
    ezgl::point2d from;
    IntersectionIdx toIdx;
    ezgl::point2d to;
    StreetSegmentIdx segmentIndex;
    bool reverse;
    int step;
};

struct PathInstruction
{
    string instruction;
    double timeTaken;
    bool turnInstruction;
};


// draw path over roads on the map
void drawPath(ezgl::renderer *g, vector<PathSegment>& path, double lineWidth, ezgl::color colour, double scaling, int highlightStep, vector<MapFeature*>& segmentIdxToFeatures);

//create travel directions
vector<PathInstruction> getTravelDirections(vector<PathSegment>& path);
double getSignedTurnProjection(StreetSegmentIdx src, StreetSegmentIdx dst, IntersectionIdx commonIntersection);

//generate path
vector<PathSegment> generateFatestPath(IntersectionIdx from, IntersectionIdx to);

vector<PathSegment> getPathSegments(vector<StreetSegmentIdx>& segmentPath); //converts vector of street segments to vector of path segments

ezgl::color getPathColour(); //getter function to return the current path colour