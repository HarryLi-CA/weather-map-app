#include "ezgl/point.hpp"
#include "ezgl/rectangle.hpp"

bool intersecting(ezgl::rectangle rectA, ezgl::rectangle rectB)
{
    bool outside = rectA.left() > rectB.right() || rectA.right() < rectB.left() || rectA.top() < rectB.bottom() || rectA.bottom() > rectB.top();
    return !outside;
}

ezgl::point2d midpoint(ezgl::point2d A, ezgl::point2d B)
{
    return {0.5*(A.x + B.x), 0.5*(A.y + B.y)};
}