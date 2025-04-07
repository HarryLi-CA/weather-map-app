#include <vector>
#include <math.h>
#include <unordered_set>
#include "SimAnnealing.h"
#include "CourierProblemInfo.h"
#include "m4Helper.h"
#include "2opt.h"
#include <atomic>
using namespace std;


const double START_TEMP = 10000;
const double END_TEMP = 1e-3;
const double COOLING_RATE = 0.995;
const int MAX_ITER = 100;


//swaoneighbourSolution
vector<int> neighbourSolution(vector<int> route, const CourierProblemInfo& cPI) {
    int i = rand() % route.size(), j = rand() % route.size();
    if (i > j) swap(i, j);

    swap(route[i], route[j]);

    if(validPath(route, cPI))
        return route;
    return {};
}

vector<int> simulatedAnnealing(vector<int> &bestRoute
, const CourierProblemInfo& cPI, atomic<bool> &forceQuit) {
    double bestCost = getTravelTime(bestRoute, cPI);
    vector<int> currRoute = bestRoute;
    double currCost = bestCost;
    double temp = START_TEMP;

    while (temp > END_TEMP && !forceQuit) {
        for (int i = 0; i < MAX_ITER && !forceQuit; i++){
                // cout<<"tests12345"<<endl;
            vector<int> newRoute = neighbourSolution(currRoute, cPI);
                // cout<<"testskjdg"<<endl;
            if(newRoute.size() > 0)
            {
                // cout<<"test"<<endl;
                double newCost = getTravelTime(newRoute, cPI);
                // cout<<"test2"<<endl;
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
                // cout<<"test3"<<endl;
        }
        temp *= COOLING_RATE; // Reduce temperature
                // cout<<"test4"<<endl;
    }
    forceQuit = true;
    cout << temp << endl;
    return bestRoute;
}