#include "ezgl/point.hpp"
#include "ezgl/rectangle.hpp"


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