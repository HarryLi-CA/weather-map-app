#pragma once
#include "ezgl/point.hpp"
#include "ezgl/rectangle.hpp"
#include "pointMath.h"
#include <vector>
#include <utility>
#include <iostream>
using namespace std;

template <typename DataType> struct QuadNode {
    ezgl::point2d point;
    ezgl::rectangle bounds;
    DataType* data;
    int sortingIndex;
};

//structure to store quad values;
template <typename DataType> struct Quad {
    ezgl::rectangle bounds;
    vector<pair<DataType*, int>> data = {};
    Quad<DataType>* topLeft;
    Quad<DataType>* topRight;
    Quad<DataType>* bottomLeft;
    Quad<DataType>* bottomRight;
};


//Stores 2-dimensional quads in a recursive pattern for quick access 
template <typename DataType> class QuadTree {
    Quad<DataType>* root;
    double unitArea;
    int size = 0;
    public:
    QuadTree(){}
    QuadTree(ezgl::rectangle bounds, double _unitArea)
    {
        Quad<DataType>* newRoot = new Quad<DataType>({bounds,{},nullptr,nullptr,nullptr,nullptr}); 
        root = newRoot;
        unitArea = _unitArea;
        cout << "initializing at " << "{" << root->bounds.top_left().x << ", " << root->bounds.top_left().y << "}, {" << root->bounds.bottom_right().x << ", " << root->bounds.bottom_right().y << "}: " << root->bounds.width() << endl;

    }
    void insert(QuadNode<DataType> node)
    {
        insert(root, node);
    }
    void subdivide(Quad<DataType>* quad)
    {
        ezgl::rectangle topLeftBounds = {quad->bounds.top_left(),quad->bounds.center()};
        ezgl::rectangle topRightBounds = {quad->bounds.top_right(),quad->bounds.center()};
        ezgl::rectangle bottomLeftBounds = {quad->bounds.bottom_left(),quad->bounds.center()};
        ezgl::rectangle bottomRightBounds = {quad->bounds.bottom_right(),quad->bounds.center()};
        Quad<DataType>* topLeftQuad = new Quad<DataType>({topLeftBounds,{},nullptr,nullptr,nullptr,nullptr});
        Quad<DataType>* topRightQuad = new Quad<DataType>({topRightBounds,{},nullptr,nullptr,nullptr,nullptr});
        Quad<DataType>* bottomLeftQuad = new Quad<DataType>({bottomLeftBounds,{},nullptr,nullptr,nullptr,nullptr});
        Quad<DataType>* bottomRightQuad = new Quad<DataType>({bottomRightBounds,{},nullptr,nullptr,nullptr,nullptr});
        //cout << "area: " << topLeftBounds.area() << endl;
        quad->topLeft = topLeftQuad;
        quad->topRight = topRightQuad;
        quad->bottomLeft = bottomLeftQuad;
        quad->bottomRight = bottomRightQuad;
        if(quad->bottomRight == nullptr)
        {
            cout << "failed!" << endl;
        }
    }
    bool isLeaf(Quad<DataType>* quad)
    {
        //since all quad values are set at the same time, if one of them is null, they are all null
        return quad->topLeft == nullptr || quad->topRight == nullptr || quad->bottomLeft == nullptr || quad->bottomRight == nullptr;
    }
    void insert(Quad<DataType>* quad, QuadNode<DataType> node)
    {
        if(quad == root){
            //cout << "inserting: " << node.point.x << ", " << node.point.y << ": " << quad->bounds.left() << ", " << quad->bounds.right() << ", " << quad->bounds.width() << endl;
        }
        if(!quad->bounds.contains(node.point)){return;} //if point is not in quad, stop function
        if(quad->bounds.area() <= unitArea)
        {
            size++;
            //cout << 100*size/(double)1153728 << "%" << endl;
            quad->data.push_back({node.data, node.sortingIndex});
            return;
        }
        //if quad is not a leaf
        if(quad->bounds.area() > unitArea)
        {
            if(isLeaf(quad)){
                subdivide(quad);
                } //subdivide leaves
            
            //attempt insertion into each tree
            insert(quad->topLeft, node);
            insert(quad->topRight, node);
            insert(quad->bottomLeft, node);
            insert(quad->bottomRight, node);
        }
        
    }
    void getInRange(ezgl::rectangle searchBounds, int layer, vector<DataType*>* totalData)
    {
        getInRange(root,searchBounds,layer,totalData);
    }

    void getQuadsInRange(ezgl::rectangle searchBounds, vector<Quad<DataType>*>* totalData)
    {
        getQuadsInRange(root, searchBounds, totalData);
    }

    void getQuadsInRange(Quad<DataType>* quad, ezgl::rectangle searchBounds, vector<Quad<DataType>*>* totalData)
    {
        // cout << "checking" << endl;
        // cout << searchBounds.area() << endl;
        //cout << (quad == nullptr) << endl;
        if(!intersecting(searchBounds,quad->bounds)){return;}
        //cout << "checking is leaf" << endl;
        if(isLeaf(quad))
        {
            totalData->push_back(quad);
            return;
        }else
        {
            //cout << "getting others in range: " << endl;
            //cout << "top right" << endl;
            getQuadsInRange(quad->topRight,searchBounds,totalData);
            getQuadsInRange(quad->topLeft,searchBounds,totalData);
            getQuadsInRange(quad->bottomRight,searchBounds,totalData);
            getQuadsInRange(quad->bottomLeft,searchBounds,totalData);
        }

    }

    void getInRange(Quad<DataType>* quad, ezgl::rectangle searchBounds, int sortingIndex, vector<DataType*>* totalData)
    {
        // cout << "checking" << endl;
        // cout << searchBounds.area() << endl;
        // cout << (quad == nullptr) << endl;
        // cout << quad->bounds.area() << endl;
        if(!intersecting(searchBounds,quad->bounds)){return;}
        //cout << "checking is leaf" << endl;
        if(isLeaf(quad))
        {
            for(auto nodeData = quad->data.begin(); nodeData < quad->data.end(); nodeData++)
            {
                totalData->push_back(nodeData->first);
            }
        }else
        {
            //cout << "getting others in range: " << endl;
            //cout << "top right" << endl;
            getInRange(quad->topRight,searchBounds,sortingIndex,totalData);
            getInRange(quad->topLeft,searchBounds,sortingIndex,totalData);
            getInRange(quad->bottomRight,searchBounds,sortingIndex,totalData);
            getInRange(quad->bottomLeft,searchBounds,sortingIndex,totalData);
        }

    }
};