#pragma once
#include "ezgl/point.hpp"
#include "ezgl/rectangle.hpp"
#include <vector>
using namespace std;

ezgl::point2d midpoint(ezgl::point2d A, ezgl::point2d B); //helper function to return midpoint between 2 point2d objects
ezgl::point2d lerp(ezgl::point2d A, ezgl::point2d B, double scale); //helper function to linearly interpolate between 2 point2d objects
bool intersecting(ezgl::rectangle rectA, ezgl::rectangle rectB); //helper function to check if 2 rectangles are intersecting
ezgl::rectangle updateBounds(ezgl::rectangle rect, ezgl::point2d point); //helper function to expand rectangle is point is outside
ezgl::rectangle recenterRect(ezgl::rectangle rect, ezgl::point2d point); //helper function to move center of rectangle
double magnitude(ezgl::point2d P); //helper function to find the magnitude of vector P as an ezgl point
double dot(ezgl::point2d A, ezgl::point2d B); //helper function to calculate the dot product between 2 vectors as ezgl points
ezgl::point2d normalize(ezgl::point2d P); //helper function that returns direction vector parallel to vector P as an ezgl point
double getDistanceToClosest(ezgl::point2d P, vector<ezgl::point2d>& points); //helper function that returns the distance from P to the closest point in a set of points