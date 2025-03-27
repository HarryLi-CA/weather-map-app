#pragma once
#include <iostream>
#include <vector>
#include "Point.h"

using namespace std;

class Heap{
  public:
    vector<Point*> items={};
    int currentItemCount=0;
    Heap();
    void add(Point* item);
    Point* removeFirst();
  
    bool contains(const Point* item);

    int getIndex(const Point* item);
    Point* getPoint(const Point* item);
  
    void updateItem(Point* const item);
  
    int length();
  
    void sortDown(Point* const item);
  
    void sortUp(Point* const item);
    void swap(Point* const itemA, Point* const itemB);
  };