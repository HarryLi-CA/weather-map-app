#pragma once
#include <iostream>
#include <vector>
#include "PathFinding.h"
#include "Point.h"

using namespace std;

class Heap{
  public:
    vector<Point*> items={};
    int currentItemCount=0;
    Heap();
    void add(Point* item);
    Point* removeFirst();
  
    bool contains(Point* item);

    int getIndex(Point* item);
    Point* getPoint(Point* item);
  
    void updateItem(Point* item);
  
    int length();
  
    void sortDown(Point* item);
  
    void sortUp(Point* item);
    void swap(Point* itemA, Point* itemB);
  };