
#include <iostream>
#include "SegmentTree.h"

using namespace std;

MinSegmentTree::MinSegmentTree(int size){
    maxArrSize = size;
    curSize = 0;
    tree.resize(4 * size, make_pair(DBL_MAX, nullptr));
}
MinSegmentTree::~MinSegmentTree(){}
void MinSegmentTree::build(vector<Point*> a){
    vector<pair<int,Point*>> b;
    for (int i = 0; i < a.size(); i++)
    {
        b.push_back(make_pair(a[i]->gCost,a[i]));
    }
    build(b);
}

void MinSegmentTree::build(vector<pair<int, Point*>> a){
    build(a, 1, 0, maxArrSize-1);
}
void MinSegmentTree::build(vector<pair<int, Point*>> a, int treeIndex, int treeLeft, int treeRight){
    if (treeLeft == treeRight) { //end of tree
        if(treeRight < a.size()){
            tree[treeIndex] = a[treeLeft];
            a[treeLeft].second->segTreeIndex = treeIndex;
            curSize++;
        }
    } else {
        int treeMid = (treeLeft + treeRight) / 2;
        build(a, treeIndex*2, treeLeft, treeMid);
        if(treeRight < a.size())
            build(a, treeIndex*2+1, treeMid+1, treeRight);
        tree[treeIndex] = min(tree[treeIndex*2], tree[treeIndex*2 + 1]);
    }
}
int getBaseIndex(Point* point){
    return point->segTreeIndex;
}

// void MinSegmentTree::removeLazy(int treeIndex){
//     if(isLazy[treeIndex]){
//         tree[treeIndex*2] = tree[treeIndex*2+1] = tree[treeIndex];
//         isLazy[treeIndex*2] = isLazy[treeIndex*2+1] = true;
//         isLazy[treeIndex] = false;
//     }
// }

void MinSegmentTree::push(pair<double, Point*> push_val){
    update(curSize, push_val);
    curSize++;
}

void MinSegmentTree::update(int pos, pair<double, Point*> value){
    update(1, 0, maxArrSize, pos, value);
}


void MinSegmentTree::update(int treeIndex, int treeLeft, int treeRight, int pos, pair<double, Point*> value){
    if (treeLeft == treeRight) {
        tree[treeIndex] = value; //plan to add difference
        value.second->segTreeIndex = treeIndex;
    } else {
        int treeMid = (treeLeft + treeRight) / 2;
        if (pos <= treeMid)
            update(treeIndex*2, treeLeft, treeMid, pos, value);
        else
            update(treeIndex*2+1, treeMid+1, treeRight, pos, value);
        tree[treeIndex] = min(tree[treeIndex*2], tree[treeIndex*2+1]);
    }
}

void MinSegmentTree::update(int pos, double value){
    update(1, 0, maxArrSize, pos, value);
}

void MinSegmentTree::update(int treeIndex, int treeLeft, int treeRight, int pos, double value){
    if (treeLeft == treeRight) {
        tree[treeIndex].first = value;
    } else {
        int treeMid = (treeLeft + treeRight) / 2;
        if (pos <= treeMid)
            update(treeIndex*2, treeLeft, treeMid, pos, value);
        else
            update(treeIndex*2+1, treeMid+1, treeRight, pos, value);
        tree[treeIndex] = min(tree[treeIndex*2], tree[treeIndex*2+1]);
    }
}

pair<double, Point*> MinSegmentTree::query(){
    //cout << query(1, 0, maxArrSize-1, 0, maxArrSize-1).first <<endl;
    return query(1, 0, maxArrSize-1, 0, maxArrSize-1);
}
pair<double, Point*> MinSegmentTree::query(int treeIndex, int treeLeft, int treeRight, int left, int right){
    if (left > right){
        //cout<<"error: segtree parameters wrong"<<endl;
        return make_pair(DBL_MAX, nullptr);
    }
    if (left == treeLeft && treeRight == right){
        return tree[treeIndex];
    }
    // removeLazy(treeIndex);
    int treeMid = (treeLeft + treeRight) / 2;
    pair<double, Point*> lQuery = query(treeIndex*2, treeLeft, treeMid, left, min(right, treeMid));
    pair<double, Point*> rQuery = query(treeIndex*2+1, treeMid+1, treeRight, max(left, treeMid+1), right);

    // if (lQuery.first != DBL_MAX)
    // {
    //    cout << lQuery.first<<endl;
    // }
    // else if(rQuery.first != DBL_MAX){
    //     cout<<rQuery.first<<endl;
    // }
    
    return min(lQuery, rQuery);
}