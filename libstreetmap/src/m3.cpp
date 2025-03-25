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
#include "Point.h"
#include <vector>
#include <random>
#include <stdio.h>
#include <cstdlib>
#include <ctime>

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
const double kTurnThreshold = 0.6; //absolute sinusoidal coefficient required for an angle to be considered a turn 
const double kHighwaySpeedLimit = 80*1000.0/3600.0; //minimum speed required for highway terminiology to be used in instructions
const ezgl::color kPathColour = {224, 255, 133}; //colour of a given path
const ezgl::color kHighlightedPathColour = {133, 133, 244}; //colour of a given path
const ezgl::color kColorPathStreetName = {50, 50, 255, 255};
const ezgl::color kColorHighlightedPathStreetName = {255, 255, 244}; //colour of a given path

void drawPath(ezgl::renderer *g, vector<PathSegment>& path, double lineWidth, ezgl::color colour, double scaling, int highlightStep, vector<MapFeature*>& segmentIdxToFeatures)
{
    //dont draw empthy paths
    if(path.size() == 0){return;}
    //get end points
    ezgl::point2d startPoint = path[0].from;
    ezgl::point2d endPoint = path[path.size() - 1].to;
    vector<ezgl::point2d> arrowPositions;


    //set border colour
    ezgl::color borderColour = staticRGBScale(colour, kPathBorderBrightness);
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

        ezgl::color pathColour = colour;

        if(highlightStep == pathSeg.step)
        {
            pathColour = kHighlightedPathColour;
        }

        //set path colour
        g->set_color(pathColour);

        for(int pointIndex = 0; pointIndex < points->size() - 1; pointIndex++)
        {
            ezgl::point2d from = LatLonToPoint2D((*points)[pointIndex]);
            ezgl::point2d to = LatLonToPoint2D((*points)[pointIndex+1]);
            g->draw_line(from, to);
        }
    }

    //draw endpoints
    g->set_color(colour);
    g->fill_arc(startPoint,endPointRadius,0.0,360.0); //draw start point
    g->fill_arc(endPoint,endPointRadius,0.0,360.0); //draw end point

    //draw arrows
    for(int pathIndex = 0; pathIndex < path.size(); pathIndex++)
    {
        PathSegment pathSeg = path[pathIndex];
        vector<LatLon>* points = getStreetSegmentPoints(pathSeg.segmentIndex);
        
        //set triangle colour
        ezgl::color arrowColour = staticRGBScale(colour, kArrowColourBrightness);
        //draw triangle
        ezgl::point2d firstPoint = LatLonToPoint2D((*points)[points->size()/2 - 1]);
        ezgl::point2d secondPoint = LatLonToPoint2D((*points)[points->size()/2]);

        ezgl::point2d deltaPosition = normalize(secondPoint - firstPoint); //get direction vector of path

        if(pathSeg.reverse){deltaPosition = {-deltaPosition.x,-deltaPosition.y};} //if segment is reversed, display arrows in reverse direction

        ezgl::point2d middlePoint = midpoint(firstPoint,secondPoint); //point where the arrow will be drawn
        double arrowDistance = getDistanceToClosest(middlePoint,arrowPositions);//get distance to closest arrow
        double arrowSize = kPathArrowAbsoluteSize;
        arrowSize = min(lineWidth, arrowSize);

        if(arrowDistance > kMinimumArrowDistance+2*arrowSize) //account for the size of arrows (approximate as circles) when calculating distance
        {
            drawTriangle(g,middlePoint,deltaPosition,arrowColour,arrowSize);

            arrowPositions.push_back(middlePoint);
        }

        //draw street names
        StreetIdx streetID = getStreetSegmentInfo(pathSeg.segmentIndex).streetID;
        if (streetID < osmhelp::getNumStreets() && streetID > 0)
        {
            MapFeature* feature = segmentIdxToFeatures[pathSeg.segmentIndex];
            int longestIndex = feature->longestPointSegmentIndex;
            //get longest segment points
            cout << feature->points->size() << endl;
            ezgl::point2d fromPoint = (*feature->points)[longestIndex];
            ezgl::point2d toPoint = (*feature->points)[longestIndex + 1];

            //get point in the middle of longest segment
            ezgl::point2d midPoint = midpoint(fromPoint, toPoint);
            //get displacement vector between points
            ezgl::point2d delta = {toPoint.x - fromPoint.x, toPoint.y - fromPoint.y};
            double angle = (atan2(delta.y, delta.x)) / kDegreeToRadian; //angle from horizontal axis (counter clockwise) of street segment
            if (angle > 90)
            {
            angle -= 180;
            }
            if (angle < -90)
            {
            angle += 180;
            }
            string streetName = getStreetName(feature->streetID);

            ezgl::color streetNameColour = kColorPathStreetName;
            
            if(highlightStep == pathSeg.step) //change colour on a highlighted street segment
            {
                streetNameColour = kColorHighlightedPathStreetName;
            }

            //roate and draw text
            g->set_color(streetNameColour);
            g->set_text_rotation(angle);
            g->draw_text(midPoint, streetName, feature->longestSegmentLength, lineWidth * kPathDisplayScaling);
        }
    }

    
    
}

vector<PathSegment> generateFatestPath(IntersectionIdx from, IntersectionIdx to)
{
    vector<PathSegment> path;

    vector<StreetSegmentIdx> segmentPath = findPathBetweenIntersections(kDefaultTurnPenalty,{from, to});
    
    path = getPathSegments(segmentPath);

    return path;
}
    
vector<PathSegment> getPathSegments(vector<StreetSegmentIdx>& segmentPath)
{
    vector<PathSegment> path;
    
    //if intersection path is empty (or invalid) return empty path
    if(segmentPath.size() == 0){return path;}

    for(int pathIndex = 0; pathIndex < segmentPath.size(); pathIndex++)
    {
        StreetSegmentInfo segmentInfo = getStreetSegmentInfo(segmentPath[pathIndex]);
        

        IntersectionIdx fromIntersection = segmentInfo.from;
        IntersectionIdx toIntersection = segmentInfo.to;
        bool reverse = false;

        if(pathIndex < segmentPath.size() - 1) //check path segfment to see if segment should be evaluated in reverse
        {
            StreetSegmentInfo nextSegmentInfo = getStreetSegmentInfo(segmentPath[pathIndex+1]);
            if(nextSegmentInfo.from == segmentInfo.from || nextSegmentInfo.to == segmentInfo.from)
            {
                fromIntersection = segmentInfo.to;
                toIntersection = segmentInfo.from;
                reverse = true;
            }
        }
        else //if segment is the final one, check previous segment
        {
            StreetSegmentInfo prevSegmentInfo = getStreetSegmentInfo(segmentPath[pathIndex-1]);
            if(prevSegmentInfo.from == segmentInfo.to || prevSegmentInfo.to == segmentInfo.to)
            {
                fromIntersection = segmentInfo.to;
                toIntersection = segmentInfo.from;
                reverse = true;
            }
        }

        ezgl::point2d from = LatLonToPoint2D(getIntersectionPosition(fromIntersection));
        ezgl::point2d to = LatLonToPoint2D(getIntersectionPosition(toIntersection));

        StreetSegmentIdx commonSegment = segmentPath[pathIndex];
    
        PathSegment pathSegment = {fromIntersection,from,toIntersection,to,commonSegment,reverse, -1};
        path.push_back(pathSegment);
    }

    getTravelDirections(path);
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
    cout <<pathTravelTime << endl;
    return pathTravelTime;
}

std::vector<StreetSegmentIdx> findPathBetweenIntersections(const double turn_penalty, const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids)
{
    //cout << "Started Finding Path " << intersect_ids.first << " " << intersect_ids.second <<"\n";
    // clock_t startTime = clock();
    PathFinder pF; pF.bidirectionalEasy = false;
    vector<Point*> pointVector = pF.calculatePathArray(turn_penalty, intersect_ids);
    vector<StreetSegmentIdx> streetIds;
    if(pointVector.size() > 0){
        for (int i = 0; i < pointVector.size(); i++)
        {
            if(pointVector[i]->segId >= 0)
                streetIds.push_back(pointVector[i]->segId);
            //delete pointVector[i];
        }
    }

    return streetIds;
}

ezgl::color getPathColour()
{
    return kPathColour;
}

vector<PathInstruction> getTravelDirections(vector<PathSegment>& path)
{
    if(path.size() < 2){return {};}

    int step = 0;

    vector<PathInstruction> travelDirections;


    StreetIdx startingRoad = getStreetSegmentInfo(path[0].segmentIndex).streetID;
    
    string lastRoadName = getStreetName(startingRoad);
    double runningPathLength = 0;
    double runningTimeTaken = 0;

    for(int pathIndex = 0; pathIndex < path.size(); pathIndex++)
    {
        PathSegment& pathSegment = path[pathIndex];

        StreetIdx streetID = getStreetSegmentInfo(pathSegment.segmentIndex).streetID;
        
        string currentRoadName = getStreetName(streetID);
        
        if(pathIndex > 0)
        {
            PathSegment lastSegment = path[pathIndex - 1];
            double projection = getSignedTurnProjection(lastSegment.segmentIndex, pathSegment.segmentIndex, lastSegment.toIdx);
            if(abs(projection) > kTurnThreshold) //significant turn projection should to conveyed as a left or right turn
            {
                //finalize step from old road
                int totalLength = (int)runningPathLength;
                string unit = "meters";

                if(totalLength >= 1000)
                {
                    totalLength /= 1000;
                    unit = "km";
                }


                string straightDirections = "Travel " + to_string(totalLength) + " " + unit;
                
                if(lastRoadName.find('<') == string::npos)
                {
                    straightDirections += " along " + lastRoadName;
                }
                

                travelDirections.push_back({straightDirections, runningTimeTaken, false});
                step++; //set step number to next step

                string turnDirection = "left";

                if(projection < 0) //if turn projection is less than 0, it is a right turn
                {
                    turnDirection = "right";
                }

                string turnInstruction = "Make a " + turnDirection + " turn";
                if(currentRoadName != lastRoadName && currentRoadName.find('<') == string::npos)
                {
                    turnInstruction += " onto " + currentRoadName;
                }

                travelDirections.push_back({turnInstruction, kDefaultTurnPenalty, true});
                step++; //set step number to next step

                runningPathLength = 0;
                runningTimeTaken = 0;
            }
            else if(lastRoadName != currentRoadName && currentRoadName.find('<') == string::npos)
            {
                //bool merging = getStreetSegmentInfo(pathSegment.segmentIndex).speedLimit > kHighwaySpeedLimit; //is new path a highway
                //bool exiting = getStreetSegmentInfo(lastSegment.segmentIndex).speedLimit > kHighwaySpeedLimit; //is old path a highway

                //finalize step from old road
                int totalLength = (int)runningPathLength;
                string unit = "meters";

                if(totalLength >= 1000)
                {
                    totalLength /= 1000;
                    unit = "km";
                }


                string straightDirections = "Travel " + to_string(totalLength) + " " + unit;
                
                if(lastRoadName.find('<') == string::npos)
                {
                    straightDirections += " along " + lastRoadName;
                }
                

                travelDirections.push_back({straightDirections, runningTimeTaken, false});
                step++; //set step number to next step

                //add step to enter new road
                string mergeInstruction = "Enter " + currentRoadName;
                travelDirections.push_back({mergeInstruction, 0, true});
                step++;
                
                runningPathLength = 0;
                runningTimeTaken = 0;

            }
        }

        pathSegment.step = step; //set the step ID of the path segment to the current step
        
        double pathLength = findStreetSegmentLength(pathSegment.segmentIndex);
        double pathTime = findStreetSegmentTravelTime(pathSegment.segmentIndex);
        runningPathLength += pathLength;
        runningTimeTaken += pathTime;
        lastRoadName = currentRoadName;
        
        
    }

    int totalLength = (int)runningPathLength;
    string unit = "meters";

    if(totalLength >= 1000)
    {
        totalLength /= 1000;
        unit = "km";
    }


    string straightDirections = "Travel " + to_string(totalLength) + " " + unit;
    
    if(lastRoadName.find('<') == string::npos)
    {
        straightDirections += " along " + lastRoadName;
    }

    travelDirections.push_back({straightDirections, round(runningTimeTaken), false});

    return travelDirections;
}

double getSignedTurnProjection(StreetSegmentIdx src, StreetSegmentIdx dst, IntersectionIdx commonIntersection) //-1 = right, 1 = left
{
    vector<LatLon> srcPoints = *getStreetSegmentPoints(src);
    vector<LatLon> dstPoints = *getStreetSegmentPoints(dst); 

    StreetSegmentInfo srcInfo = getStreetSegmentInfo(src);
    StreetSegmentInfo dstInfo = getStreetSegmentInfo(dst);
    
    ezgl::point2d srcDir = {0, 0};
    ezgl::point2d dstDir = {0, 0};

    if(srcInfo.to == commonIntersection)
    {
        srcDir = LatLonToPoint2D(srcPoints[srcPoints.size() - 1]) - LatLonToPoint2D(srcPoints[srcPoints.size() - 2]);
        srcDir = normalize(srcDir);
    }
    else
    {
        srcDir = LatLonToPoint2D(srcPoints[0]) - LatLonToPoint2D(srcPoints[1]);
        srcDir = normalize(srcDir);
    }

    if(dstInfo.to == commonIntersection)
    {
        dstDir = LatLonToPoint2D(dstPoints[dstPoints.size() - 2]) - LatLonToPoint2D(dstPoints[dstPoints.size() - 1]);
        dstDir = normalize(dstDir);
    }
    else
    {
        dstDir = LatLonToPoint2D(dstPoints[1]) - LatLonToPoint2D(dstPoints[0]);
        dstDir = normalize(dstDir);
    }
    
    double srcAngle = atan2(srcDir.y, srcDir.x);
    double dstAngle = atan2(dstDir.y, dstDir.x);
    double projectedAngle = dstAngle - srcAngle;

    return sin(projectedAngle);

}