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


//Stores 2-dimensional quads in a recursive pattern for spatial indexing 
template <typename DataType> class QuadTree {
    Quad<DataType>* root; //root quad of quad tree
    double unitArea; //threshold of quad area to be considered a leaf quad
    int size = 0; //number of data elements in quad tree
    int nodes = 1; //number of quads in quad tree
    public:
    QuadTree(){}

    void reset(ezgl::rectangle bounds) //resets tree and recreates root
    {
        Quad<DataType>* newRoot = new Quad<DataType>({bounds,{},nullptr,nullptr,nullptr,nullptr}); //create new root
        root = newRoot; //assign new root to object
    }

    void clear() //empties quad tree
    {
        if(root == nullptr){return;} //guard against double freeing
        clear(root); //clear root
        root = nullptr;
        nodes = 0;
        size = 0;
        cout << "done clearing" << endl;
    }

    void clear(Quad<DataType>* quad) //deletes quad and all of its children
    {
        if(!isLeaf(quad))
        {
            clear(quad->topRight);
            clear(quad->topLeft);
            clear(quad->bottomRight);
            clear(quad->bottomLeft);
            //cout << "clearing node" << endl;
        }
        delete quad;
    }
    QuadTree(ezgl::rectangle bounds, double _unitArea)
    {
        reset(bounds);
        unitArea = _unitArea; //assign unit area
    }
    void insert(QuadNode<DataType> node) //override function to insert node from root
    {
        insert(root, node); //insert node from root
    }

    void insertBounds(QuadNode<DataType> node) //override function to insert node from root using bounds
    {
        insertBounds(root, node); //insert node from root
    }

    void subdivide(Quad<DataType>* quad) //splits quad into 4 equally size quads and adds them as children
    {
        ezgl::rectangle topLeftBounds = {quad->bounds.top_left(),quad->bounds.center()}; //bound of top-left quad
        ezgl::rectangle topRightBounds = {quad->bounds.top_right(),quad->bounds.center()}; //bound of top-right quad
        ezgl::rectangle bottomLeftBounds = {quad->bounds.bottom_left(),quad->bounds.center()}; //bound of bottom-left quad
        ezgl::rectangle bottomRightBounds = {quad->bounds.bottom_right(),quad->bounds.center()}; //bound of bottom-right quad
        //create new subdivided quads as leaf quads
        Quad<DataType>* topLeftQuad = new Quad<DataType>({topLeftBounds,{},nullptr,nullptr,nullptr,nullptr}); //top-left quad object
        Quad<DataType>* topRightQuad = new Quad<DataType>({topRightBounds,{},nullptr,nullptr,nullptr,nullptr}); //top-right quad object
        Quad<DataType>* bottomLeftQuad = new Quad<DataType>({bottomLeftBounds,{},nullptr,nullptr,nullptr,nullptr}); //bottom-left quad object 
        Quad<DataType>* bottomRightQuad = new Quad<DataType>({bottomRightBounds,{},nullptr,nullptr,nullptr,nullptr}); //bottom-right quad object
        //assign quads as children
        quad->topLeft = topLeftQuad;
        quad->topRight = topRightQuad;
        quad->bottomLeft = bottomLeftQuad;
        quad->bottomRight = bottomRightQuad;
        nodes += 4;
    }
    bool isLeaf(Quad<DataType>* quad) //checks if quad is a leaf quad (has no children)
    {
        return quad->topLeft == nullptr || quad->topRight == nullptr || quad->bottomLeft == nullptr || quad->bottomRight == nullptr;
    }
    void insert(Quad<DataType>* quad, QuadNode<DataType> node) //insert node into leaf quad that contains the node's position
    {
        if(!quad->bounds.contains(node.point)){return;} //if point is not in quad, stop function
        //check if the quad is small enough to be a leaf quad
        if(quad->bounds.area() <= unitArea)
        {
            size++;
            //cout << 100*size/(double)1153728 << "%" << endl;
            quad->data.push_back({node.data, node.sortingIndex});
            return;
        }
        //if quad is not small enough, it cannot be a leaf quad
        if(quad->bounds.area() > unitArea)
        {
            //check if quad has no children
            if(isLeaf(quad))
            {
                subdivide(quad); //add children to quad
            } 
            
            //attempt insertion into each sub-quad
            insert(quad->topLeft, node);
            insert(quad->topRight, node);
            insert(quad->bottomLeft, node);
            insert(quad->bottomRight, node);
        }
        
    }
    void insertBounds(Quad<DataType>* quad, QuadNode<DataType> node) //insert node into leaf quad that intersects with the node's bounds
    {
        if(!intersecting(quad->bounds,node.bounds)){return;} //if bounds are not in quad, stop function
        //check if the quad is small enough to be a leaf quad
        if(quad->bounds.area() <= unitArea)
        {
            size++;
            //cout << 100*size/(double)1153728 << "%" << endl;
            quad->data.push_back({node.data, node.sortingIndex});
            return;
        }
        //if quad is not small enough, it cannot be a leaf quad
        if(quad->bounds.area() > unitArea)
        {
            //check if quad has no children
            if(isLeaf(quad))
            {
                subdivide(quad); //add children to quad
            } 
            
            //attempt insertion into each sub-quad
            insertBounds(quad->topLeft, node);
            insertBounds(quad->topRight, node);
            insertBounds(quad->bottomLeft, node);
            insertBounds(quad->bottomRight, node);
        }
        
    }

    void getInRange(ezgl::rectangle searchBounds, int layer, vector<DataType*>* totalData) //helper function to get all elements in a rectangular bound from the root
    {
        cout << "searching with root size " << root->bounds.area() << endl;
        getInRange(root,searchBounds,layer,totalData);
    }

    void getQuadsInRange(ezgl::rectangle searchBounds, vector<Quad<DataType>*>* totalData) //helper function to get all quads in a rectangular bound from the root
    {
        getQuadsInRange(root, searchBounds, totalData);
    }

    void getQuadsInRange(Quad<DataType>* quad, ezgl::rectangle searchBounds, vector<Quad<DataType>*>* totalData) //helper function to get leaf quads in a rectangular bound
    {
        //check if quad is not in range
        if(!intersecting(searchBounds,quad->bounds)){return;} //if quad is not in the specified range, do not process it
        //check if quad is a leaf quad (has no children)
        if(isLeaf(quad))
        {
            totalData->push_back(quad); //add quad to running total
            return;
        }else
        {
            //check if each sub-quad (child) is in range
            getQuadsInRange(quad->topRight,searchBounds,totalData);
            getQuadsInRange(quad->topLeft,searchBounds,totalData);
            getQuadsInRange(quad->bottomRight,searchBounds,totalData);
            getQuadsInRange(quad->bottomLeft,searchBounds,totalData);
        }

    }

    void getInRange(Quad<DataType>* quad, ezgl::rectangle searchBounds, int sortingIndex, vector<DataType*>* totalData)
    {
        //check if quad is not in range
        if(!intersecting(searchBounds,quad->bounds)){return;} //if quad is not in the specified range, do not process it
        //check if quad is a leaf quad (has no children)
        if(isLeaf(quad))
        {
            //add all data in the quad to running total
            for(auto nodeData = quad->data.begin(); nodeData < quad->data.end(); nodeData++)
            {
                totalData->push_back(nodeData->first);
            }
        }else
        {
            //check if each sub-quad is in range
            getInRange(quad->topRight,searchBounds,sortingIndex,totalData);
            getInRange(quad->topLeft,searchBounds,sortingIndex,totalData);
            getInRange(quad->bottomRight,searchBounds,sortingIndex,totalData);
            getInRange(quad->bottomLeft,searchBounds,sortingIndex,totalData);
        }

    }
};