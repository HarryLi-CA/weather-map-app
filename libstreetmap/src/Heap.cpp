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

bool Heap::contains(Point* item){
    return getIndex(item)>=0;
}

int Heap::getIndex(Point* item){
    return item->index;
}

Point* Heap::getPoint(Point* item){
    for(int i = 0; i < items.size(); i++){
        if(items[i]->segId == item->segId){ //maybe check intesection id as well
            return items[i];
        }
    }
    return nullptr;
}

void Heap::updateItem(Point* item){
    sortUp(item);
}

int Heap::length(){
    return currentItemCount;
}

void Heap::sortDown(Point* item){
    while(true){
    int parentIndex = getIndex(item);
    int childIndexLeft = parentIndex*2+1;
    int childIndexRight = parentIndex*2+2;
    int swapIndex = 0;

    if(childIndexLeft < currentItemCount){
        swapIndex = childIndexLeft;
        if(childIndexRight<currentItemCount){
        if(*items[childIndexLeft] > *items[childIndexRight]){
            swapIndex = childIndexRight;
        }
        }
        if(*items[parentIndex] > *items[swapIndex]){
            this->swap(items[parentIndex],items[swapIndex]);
        }
        else{
        return;
        }
    }
    else{
        return;
    }
    }
}

void Heap::sortUp(Point* item){
    int parentIndex = ((getIndex(item)-1)/2);
    if(parentIndex<=0){
    return;
    }
    while(parentIndex>0){
    Point* parentItem = items[parentIndex];
    if(*item<*parentItem){
        this->swap(item,parentItem);
    }
    else{
        break;
    }
    parentIndex = ((getIndex(item)-1)/2);
    }

}
void Heap::swap(Point* itemA, Point* itemB){
    int itemAIndex = getIndex(itemA);
    int itemBIndex = getIndex(itemB);
    itemB->index = itemAIndex;
    itemA->index = itemBIndex;
    items[itemAIndex] = itemB;
    items[itemBIndex] = itemA;
}
