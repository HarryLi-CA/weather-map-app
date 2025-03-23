#include "ezgl/graphics.hpp"
#include "ezgl/canvas.hpp"
#include "ezgl/application.hpp"
#include "m1.h"
#include "m3.h"
#include "renderHelper.h"
#include "pointMath.h"
#include "StreetsDatabaseAPI.h"
#include "PathFinding.h"
#include "OSMAPIHelper.h"
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
const double kPathEndpointScaling = 1.5; //scaling factor for displaying path endpoints
const double kPathArrowAbsoluteSize = 4; //scaling factor for displaying arrows
const double kMinimumArrowDistance = 20; //minimum distance between arrows to be drawn
const double kMaxArrowSize = 0; //minimum size of arrow
const double kArrowColourBrightness = 0.8; //how bright arrows are compared to the path colour
const double kPathBorderBrightness = 0.8; //how bright borders are compared to the path colour
const double kPathBorderSize = 6; //thickness of the path's borders
const ezgl::color kPathColour = {224, 255, 133}; //colour of a given path

void drawPath(ezgl::renderer *g, vector<PathSegment>& path, double lineWidth, ezgl::color colour, double scaling)
{
    //dont draw empthy paths
    if(path.size() == 0){return;}
    //get end points
    ezgl::point2d startPoint = path[0].from;
    ezgl::point2d endPoint = path[path.size() - 1].to;
    vector<ezgl::point2d> arrowPositions;


    //set border colour
    ezgl::color borderColour = {colour.red*kPathBorderBrightness,colour.green*kPathBorderBrightness,colour.blue*kPathBorderBrightness};
    g->set_color(borderColour);

    //draw background
    g->set_line_width(lineWidth*kPathDisplayScaling + kPathBorderSize);
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
    }

    //draw endpoint backgrounds
    double endPointRadius = (lineWidth/scaling)*kPathDisplayScaling*kPathEndpointScaling;
    double borderRadius = kPathBorderSize/(scaling*0.5*M_PI);
    g->fill_arc(startPoint,endPointRadius+borderRadius,0.0,360.0); //draw start point
    g->fill_arc(endPoint,endPointRadius+borderRadius,0.0,360.0); //draw end point

    //set path colour
    g->set_color(colour);

    //draw path
    g->set_line_width(lineWidth*kPathDisplayScaling);
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
    }

    //draw endpoints
    g->fill_arc(startPoint,endPointRadius,0.0,360.0); //draw start point
    g->fill_arc(endPoint,endPointRadius,0.0,360.0); //draw end point

    //draw arrows
    for(int pathIndex = 0; pathIndex < path.size(); pathIndex++)
    {
        PathSegment pathSeg = path[pathIndex];
        vector<LatLon>* points = getStreetSegmentPoints(pathSeg.segmentIndex);
        
        //set triangle colour
        ezgl::color arrowColour = {colour.red*kArrowColourBrightness, colour.green*kArrowColourBrightness, colour.blue*kArrowColourBrightness};
        //draw triangle
        ezgl::point2d firstPoint = LatLonToPoint2D((*points)[points->size()/2]);
        ezgl::point2d secondPoint = LatLonToPoint2D((*points)[points->size()/2 + 1]);

        ezgl::point2d deltaPosition = normalize(secondPoint - firstPoint); //get direction vector of path
        ezgl::point2d middlePoint = midpoint(firstPoint,secondPoint); //point where the arrow will be drawn
        double arrowDistance = getDistanceToClosest(middlePoint,arrowPositions);//get distance to closest arrow
        double arrowSize = kPathArrowAbsoluteSize;
        arrowSize = min(lineWidth, arrowSize);

        if(arrowDistance > kMinimumArrowDistance+2*arrowSize) //account for the size of arrows (approximate as circles) when calculating distance
        {
            drawTriangle(g,middlePoint,deltaPosition,arrowColour,arrowSize);

            arrowPositions.push_back(middlePoint);
        }
    }
    
}

vector<PathSegment> generateFatestPath(IntersectionIdx from, IntersectionIdx to)
{
    vector<PathSegment> path;

    vector<StreetSegmentIdx> segmentPath = findPathBetweenIntersections(kDefaultTurnPenalty,{from, to});
    
    //if intersection path is empty (or invalid) return empty path
    if(segmentPath.size() == 0){return path;}

    for(int pathIndex = 0; pathIndex < segmentPath.size() - 1; pathIndex++)
    {
        StreetSegmentInfo segmentInfo = getStreetSegmentInfo(segmentPath[pathIndex]);

        ezgl::point2d from = LatLonToPoint2D(getIntersectionPosition(segmentInfo.from));
        ezgl::point2d to = LatLonToPoint2D(getIntersectionPosition(segmentInfo.to));

        StreetSegmentIdx commonSegment = segmentPath[pathIndex];
    
        PathSegment pathSegment = {from,to,commonSegment};
        path.push_back(pathSegment);
    }

    return path;
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
            StreetSegmentInfo currentSegmentInfo = osmhelp::getStreetSegmentInfo(segmentIndex);
            StreetSegmentInfo nextSegmentInfo = osmhelp::getStreetSegmentInfo(nextSegmentIndex);
            //if streets of segments are not the same, there is a turn
            if(currentSegmentInfo.streetID != nextSegmentInfo.streetID)
            {
                pathTravelTime += turn_penalty; //apply turn penalty
            }
        }
    }
    return pathTravelTime;
}

vector<PathSegment> generateRandomPath()
{

    srand(time(NULL));
    StreetSegmentIdx start = rand()%(getNumStreetSegments());
    vector<PathSegment> path;
    StreetSegmentIdx current = start;

    int pathSize = 100;
    for(int pathIndex = 0; pathIndex < pathSize; pathIndex++)
    {
        StreetSegmentInfo info = osmhelp::getStreetSegmentInfo(current);
        IntersectionIdx nextIntersection = info.to;
        StreetSegmentIdx next = current;
        vector<StreetSegmentIdx>* nextList = getIntersectionStreetSegments(nextIntersection);
        for(int i = 0; i < nextList->size(); i++)
        {
            StreetSegmentIdx nextIndex = (*nextList)[i];
            StreetSegmentInfo nextInfo = osmhelp::getStreetSegmentInfo(nextIndex);
            if(nextInfo.to != nextIntersection && nextIndex != current)
            {
                next = (*nextList)[i];
                break;
            }
        }
        vector<LatLon>* points = getStreetSegmentPoints(current);

        path.push_back({LatLonToPoint2D((*points)[0]), LatLonToPoint2D((*points)[points->size() - 1]), current}); 
        current = next;
    }

    return path;
}

std::vector<StreetSegmentIdx> findPathBetweenIntersections(const double turn_penalty, const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids)
{
    cout << "Started Finding Path" << endl;
    int startTime = clock();
    vector<Point*> pointVector = calculatePathArray(turn_penalty, intersect_ids);
    vector<StreetSegmentIdx> streetIds;
    delete pointVector[0];
    for (int i = 1; i < pointVector.size(); i++)
    {
        streetIds.push_back(pointVector[i]->segId);
        delete pointVector[i];
    }
    cout<<"Ended finding with: "<< clock() - startTime << endl;
    return streetIds;
}

ezgl::color getPathColour()
{
    return kPathColour;
}