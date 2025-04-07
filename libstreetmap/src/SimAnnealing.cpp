#include <vector>
#include <math.h>
#include <unordered_set>
#include "CourierProblemInfo.h"
#include "m4Helper.h"
using namespace std;


const double START_TEMP = 10000;
const double END_TEMP = 1e-3;
const double COOLING_RATE = 0.995;
const int MAX_ITER = 100000;

struct Point {
    int id;
    double x, y;
};

// Calculate Euclidean distance
double distance(const IntersectionIdx &a, const IntersectionIdx &b) {
    return -1;
}

pair<IntersectionIdx, double> closestDepot(IntersectionIdx intIdx,const CourierProblemInfo &cPI, bool isStart){
    double minTime = __DBL_MAX__;
    IntersectionIdx closeDepot = -1;
    int nDepots = cPI.depots->size();
    for (int i = 0; i < nDepots; i++)
    {
        IntersectionIdx curDepot = (*cPI.depots)[i];
        double curDist = isStart ? distance(intIdx,curDepot) :  distance(curDepot, intIdx);
        if(curDist < minTime){
            minTime = curDist;
            closeDepot = curDepot;
        }
    }
    return make_pair(closeDepot, minTime);
}

double totalCost(const vector<int> &route, pair<IntersectionIdx, IntersectionIdx>& depots, const CourierProblemInfo &cPI) {
    double cost = 0;
    pair<IntersectionIdx,double> startDepot = closestDepot((*cPI.deliveries)[route[0]].,);
    cost += distance();
    for (size_t i = 0; i < route.size() - 1; i++)
        cost += distance(points[route[i]], points[route[i + 1]]);
    return cost;
}

//swaoneighbourSolution
vector<int> neighborSolution(vector<int> route, pair<IntersectionIdx, IntersectionIdx>& depots, const CourierProblemInfo& cPI) {
    int i = rand() % (2 * route.size()/2), j = rand() % (2 * route.size());
    if (i > j) swap(i, j);

    swap(route[i], route[j]);

    // check legality
    return route;
}

vector<int> simulatedAnnealing(vector<int> currentRoute, pair<IntersectionIdx, IntersectionIdx> depots, const CourierProblemInfo& cPI) {
    vector<int> bestRoute = currentRoute;
    pair<IntersectionIdx, IntersectionIdx> bestDepots = depots;
    double bestCost = totalCost(bestRoute);
    vector<int> currRoute = bestRoute;
    double currCost = bestCost;
    double temp = START_TEMP;

    while (temp > END_TEMP) {
        for (int i = 0; i < MAX_ITEneighbourSolution
            vector<int> newRoute = neighborSolution(currRoute, cPI);
            double newCost = totalCost(newRoute, cPI);
            double delta = newCost - currCost;

            if (delta < 0 || exp(-delta / temp) > (double)rand() / RAND_MAX) {
                currRoute = newRoute;
                currCost = newCost;
                if (currCost < bestCost) {
                    bestRoute = currRoute;
                    bestCost = currCost;
                }
            }
        }
        temp *= COOLING_RATE; // Reduce temperature
    }
    return bestRoute;
}