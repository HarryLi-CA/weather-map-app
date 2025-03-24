#include "ezgl/point.hpp"
#include "ezgl/rectangle.hpp"
#include <math.h>
#include <vector>
#include <cfloat>
#include "pointMath.h"


using namespace std;

bool intersecting(ezgl::rectangle rectA, ezgl::rectangle rectB)
{
    bool outside = rectA.left() > rectB.right() || rectA.right() < rectB.left() || rectA.top() < rectB.bottom() || rectA.bottom() > rectB.top();
    return !outside;
}

ezgl::point2d midpoint(ezgl::point2d A, ezgl::point2d B)
{
    return {0.5*(A.x + B.x), 0.5*(A.y + B.y)};
}

ezgl::point2d lerp(ezgl::point2d A, ezgl::point2d B, double scale)
{
    scale = min(max(scale,0.0),1.0);
    return {scale*B.x + (1 - scale)*A.x, scale*B.y + (1 - scale)*A.y,};
}
ezgl::rectangle updateBounds(ezgl::rectangle rect, ezgl::point2d point)
{
    double rightX, topY, leftX, botY; //bounds of new rectangle
    double aspectRatio = rect.width()/rect.height();

    //set bounds to current rectangle bounds
    rightX = rect.right();
    leftX = rect.left();
    topY = rect.top();
    botY = rect.bottom();

    //expand rectangle to fit point
    if(rightX < point.x){rightX = point.x;}
    if(leftX > point.x){leftX = point.x;}
    if(topY < point.y){topY = point.y;}
    if(botY > point.y){botY = point.y;}

    double currentAsepct = (rightX - leftX)/(topY - botY);
    double width = rightX - leftX;
    double height = topY - botY;
    if(currentAsepct > aspectRatio)
    {
        //adjust height
        double extraHeight = width*rect.height()/rect.width() - height;
        topY += extraHeight/2;
        botY -= extraHeight/2;
    }
    if(currentAsepct < aspectRatio)
    {
        //adjust width
        double extraWidth = height*rect.width()/rect.height() - width;
        rightX += extraWidth/2;
        leftX -= extraWidth/2;
    }

    //create new rectangle and return
    ezgl::rectangle newRect({leftX, botY}, {rightX, topY});

    return newRect;
}

ezgl::rectangle recenterRect(ezgl::rectangle rect, ezgl::point2d point)
{
   ezgl::point2d positionChange = point - rect.center(); //get change in position
   //set rect bounds to current bounds + change in position
   ezgl::rectangle newRect(rect.bottom_left() + positionChange, rect.top_right() + positionChange);
   return newRect;
}

double magnitude(ezgl::point2d P)
{
    return sqrt(P.x*P.x + P.y*P.y); //apply formula magntidue = sqrt(x^2 + y^2) for 2-dimensional vectors
}

ezgl::point2d normalize(ezgl::point2d P)
{
    double mag = magnitude(P); //get magnitude of P
    ezgl::point2d directionVector = {P.x/mag, P.y/mag}; //calculate direction vector parallel to P
    return directionVector;
}

double getDistanceToClosest(ezgl::point2d P, vector<ezgl::point2d>& points)
{
    double closestDistance = DBL_MAX;
    for(int pointIndex = 0; pointIndex < points.size(); pointIndex++)
    {
        double distance = magnitude(points[pointIndex] - P);
        closestDistance = min(distance, closestDistance);
    }
    return closestDistance;
}

double dot(ezgl::point2d A, ezgl::point2d B)
{
    return A.x*B.x + A.y*B.y;
}