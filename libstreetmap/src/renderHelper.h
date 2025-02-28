#include "ezgl/canvas.hpp";

using namespace std;

//speicifes how renderer will draw features
enum DrawType {
    POINT,
    LINE,
    POLYGON
};

class MapFeature {
    public:
    int layer; //layer to draw the feature. low layers will be drawn first.
    vector<ezgl::point2d>* points; //position of vertices of feature relative to absolute origin.
    DrawType drawType; //method renderer will use to draw feature.
    MapFeature(int _layer, vector<ezgl::point2d> *_points, DrawType _drawType);
};