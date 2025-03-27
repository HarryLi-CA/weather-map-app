#include <iostream>
#include <vector>
#include <algorithm>
#include "PathFinding.h"
#include "Heap.h"

using namespace std;

Heap::Heap(){}
void Heap::add(Point* item){
    items.push_back(item);
    item->index = currentItemCount;
    sortUp(item);
    currentItemCount++;
}
Point* Heap::removeFirst(){
    Point* firstItem = items[0];
    currentItemCount--;
    items[0] = items[items.size()-1];
    items[0]->index = 0;
    if(items[0]==firstItem){
        items.clear();
    }
    else{
    items.pop_back();
    sortDown(items[0]);
    }
    return firstItem;
}

bool Heap::contains(const Point* item){
    return getIndex(item)>=0;
}

int Heap::getIndex(const Point* item){
    return item->index;
}

Point* Heap::getPoint(const Point* item){
    for(int i = 0; i < items.size(); i++){
        if(items[i]->segId == item->segId){ //maybe check intesection id as well
            return items[i];
        }
    }
    return nullptr;
}

void Heap::updateItem(Point* const item){
    sortUp(item);
}

int Heap::length(){
    return currentItemCount;
}

void Heap::sortDown(Point* const item){
    int parentIndex = getIndex(item);
    int childIndexLeft = parentIndex*2+1;
    int childIndexRight = parentIndex*2+2;
    int& swapIndex = childIndexLeft;
    while(childIndexLeft < currentItemCount){
        if(childIndexRight<currentItemCount){
        if(*items[childIndexLeft] > *items[childIndexRight]){
            swapIndex = childIndexRight;
        }
        }
        if(*items[parentIndex] > *items[swapIndex]){
            swap(items[parentIndex],items[swapIndex]);
            parentIndex = swapIndex;
        }
        else{
        return;
        }
        childIndexLeft = parentIndex*2+1;
        childIndexRight = parentIndex*2+2;
    }
}

void Heap::sortUp(Point* const item){
    int parentIndex = ((getIndex(item)-1)/2);
    if(parentIndex<=0){
    return;
    }
    while(parentIndex>0){
    Point* parentItem = items[parentIndex];
    if(*item<*parentItem){
        swap(item,parentItem);
    }
    else{
        break;
    }
    parentIndex = ((getIndex(item)-1)/2);
    }

}
void Heap::swap(Point* const itemA, Point* const itemB){
    int itemAIndex = getIndex(itemA);
    int itemBIndex = getIndex(itemB);
    itemB->index = itemAIndex;
    itemA->index = itemBIndex;
    items[itemAIndex] = itemB;
    items[itemBIndex] = itemA;
}
