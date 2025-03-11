#pragma once
#include "ezgl/point.hpp"
#include "ezgl/rectangle.hpp"

ezgl::point2d midpoint(ezgl::point2d A, ezgl::point2d B); //helper function to return midpoint between 2 point2d objects
ezgl::point2d lerp(ezgl::point2d A, ezgl::point2d B, double scale); //helper function to linearly interpolate between 2 point2d objects
bool intersecting(ezgl::rectangle rectA, ezgl::rectangle rectB); //helper function to check if 2 rectangles are intersecting
ezgl::rectangle updateBounds(ezgl::rectangle rect, ezgl::point2d point); //helper function to expand rectangle is point is outside
ezgl::rectangle recenterRect(ezgl::rectangle rect, ezgl::point2d point); //helper function to move center of rectangle