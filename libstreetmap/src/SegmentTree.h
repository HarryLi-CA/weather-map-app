#pragma once
#include <vector>
#include <random>
#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include "Point.h"
using namespace std;
class MinSegmentTree
{
private:
public:
    vector<pair<double,Point*>> tree;
    vector<bool> isLazy;
    int maxArrSize = 0;
    int curSize = 0;
    MinSegmentTree(int size);
    ~MinSegmentTree();

    /*
    a is pair (value, pointRef)
    treeIndex is head of tree
    treeLeft is left most array value
    treeRight is right most array value
    */
    void build(vector<Point*> a);
    void build(vector<pair<int, Point*>> a);
    void build(vector<pair<int, Point*>> a, int treeIndex, int treeLeft, int treeRight);
    // void removeLazy(int treeIndex);
    void push(pair<double, Point*> push_val);
    int getBaseIndex(Point* point);
    void update(int pos, pair<double, Point*> value);
    void update(int treeIndex, int treeLeft, int treeRight, int pos, pair<double, Point*> value);

    void update(int pos, double value);
    void update(int treeIndex, int treeLeft, int treeRight, int pos, double value);
    pair<double, Point*> query();
    pair<double, Point*> query(int treeIndex, int treeLeft, int treeRight, int left, int right);
};