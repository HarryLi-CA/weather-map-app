#include "ezgl/graphics.hpp"
#include "ezgl/canvas.hpp"
#include "ezgl/application.hpp"
#include "m1.h"
#include "m3.h"
#include "renderHelper.h"
#include "pointMath.h"
#include "StreetsDatabaseAPI.h"
#include "PathFinding.h"
#include <vector>
#include <random>
#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <cfloat>

using namespace std;

const double kMinimumTurnAngle = M_PI/6.0; //minimum turn angle for street to be considered a turn
const double kDefaultTurnPenalty = 10; //penalty imposed on a turning street
const double kPathDisplayScaling = 2; //scaling factor for displaying paths
const double kPathArrowScaling = 2; //scaling factor for displaying arrows
const double kMinimumArrowDistance = 20; //minimum distance between arrows to be drawn
const double kMinimumArrowSize = 10; //minimum size of arrow

void drawPath(ezgl::renderer *g, vector<PathSegment>& path, double lineWidth, ezgl::color colour, double scaling)
{
    g->set_line_width(lineWidth*kPathDisplayScaling);
    g->set_color(colour);

    vector<ezgl::point2d> arrowPositions;

    for(int pathIndex = 0; pathIndex < path.size(); pathIndex++)
    {
        PathSegment pathSeg = path[pathIndex];
        vector<LatLon>* points = getStreetSegmentPoints(pathSeg.segmentIndex);
        for(int pointIndex = 0; pointIndex < points->size() - 1; pointIndex++)
        {
            ezgl::point2d from = LatLonToPoint2D((*points)[pointIndex]);
            ezgl::point2d to = LatLonToPoint2D((*points)[pointIndex+1]);
            g->draw_line(from, to);
        }
        
        //draw triangle
        ezgl::point2d firstPoint = LatLonToPoint2D((*points)[0]);
        ezgl::point2d secondPoint = LatLonToPoint2D((*points)[1]);

        ezgl::point2d deltaPosition = normalize(secondPoint - firstPoint); //get direction vector of path
        ezgl::point2d middlePoint = firstPoint; //point where the arrow will be drawn
        double arrowDistance = getDistanceToClosest(middlePoint,arrowPositions);
        double arrowSize = lineWidth*kPathArrowScaling;
        arrowSize = max(kMinimumArrowSize/scaling, arrowSize);

        if(arrowDistance > kMinimumArrowDistance+2*arrowSize) //account for the size of arrows (approximate as circles) when calculating distance
        {
            drawTriangle(g,middlePoint,deltaPosition,colour,arrowSize);

            arrowPositions.push_back(middlePoint);
        }
    }
}

double computePathTravelTime(const double turn_penalty, const std::vector<StreetSegmentIdx>& path)
{
    double pathTravelTime = 0;
    for(int pathIndex = 0; pathIndex < path.size(); pathIndex++)
    {
        StreetSegmentIdx segmentIndex = path[pathIndex];
        
        double travelTime = findStreetSegmentTravelTime(segmentIndex);
        pathTravelTime += travelTime; //add travel time

        if(pathIndex < path.size() - 1)
        {
            //get next segment
            StreetSegmentIdx nextSegmentIndex = path[pathIndex+1];
            //get info for both segments
            StreetSegmentInfo currentSegmentInfo = getStreetSegmentInfo(segmentIndex);
            StreetSegmentInfo nextSegmentInfo = getStreetSegmentInfo(nextSegmentIndex);
            //if streets of segments are not the same, there is a turn
            if(currentSegmentInfo.streetID != nextSegmentInfo.streetID)
            {
                pathTravelTime += turn_penalty; //apply turn penalty
            }
        }
    }
    return pathTravelTime;
}

vector<PathSegment>* generateRandomPath()
{

    srand(time(NULL));
    StreetSegmentIdx start = rand()%(getNumStreetSegments());
    vector<PathSegment>* path = new vector<PathSegment>();
    StreetSegmentIdx current = start;

    int pathSize = 100;
    for(int pathIndex = 0; pathIndex < pathSize; pathIndex++)
    {
        cout << "adding: " << current << endl;
        StreetSegmentInfo info = getStreetSegmentInfo(current);
        IntersectionIdx nextIntersection = info.to;
        StreetSegmentIdx next = current;
        vector<StreetSegmentIdx>* nextList = getIntersectionStreetSegments(nextIntersection);
        for(int i = 0; i < nextList->size(); i++)
        {
            StreetSegmentIdx nextIndex = (*nextList)[i];
            StreetSegmentInfo nextInfo = getStreetSegmentInfo(nextIndex);
            if(nextInfo.to != nextIntersection && nextIndex != current)
            {
                next = (*nextList)[i];
                break;
            }
        }
        vector<LatLon>* points = getStreetSegmentPoints(current);

        path->push_back({LatLonToPoint2D((*points)[0]), LatLonToPoint2D((*points)[points->size() - 1]), current}); 
        current = next;
    }

    return path;
}

std::vector<StreetSegmentIdx> findPathBetweenIntersections(const double turn_penalty, const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids)
{
    vector<Point> pointVector = calculatePathArray(turn_penalty, intersect_ids);
    vector<StreetSegmentIdx> streetIds;
    for (int i = 1; i < pointVector.size(); i++)
    {
        streetIds.push_back(pointVector[i].segId);
    }
    return streetIds;
}
