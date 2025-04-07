#include <vector>
#include <math.h>
#include <unordered_set>
#include "SimAnnealing.h"
#include "CourierProblemInfo.h"
#include "m4Helper.h"
#include "2opt.h"
#include <atomic>
#include <thread>
#include <mutex>
#include <algorithm>
using namespace std;


mutex annealingFlag;
const double START_TEMP = 10000;
const double END_TEMP = 1e-3;
const double COOLING_RATE = 0.995;
const int MAX_ITER = 10;

//swaoneighbourSolution
vector<int> neighbourSolution(vector<int> route, const CourierProblemInfo& cPI) {
    int i = rand() % route.size(), j = rand() % route.size();
    if (i > j) swap(i, j);

    swap(route[i], route[j]);
    //cout<< route[i] << " " << route[j] << endl;

    if(validPath(route, cPI))
        return route;
    return {};
}

vector<int> simulatedAnnealing(vector<int> &bestRoute
, const CourierProblemInfo& cPI, atomic<bool> &forceQuit, vector<DeliveryInf> deliveryCopy) {
    annealingFlag.unlock();
    //cout << "start annealing" << endl;
    double bestCost = getCheapTravelTime(bestRoute, cPI);
    //cout << "got travel time" << endl;
    vector<int> currRoute;
    vector<int> randomPickups;
    double currCost = bestCost;
    double temp = START_TEMP;

    //cout << "creating pickups" << endl;

    for(int i = 0; i < deliveryCopy.size(); i++)
    {
        randomPickups.push_back(i);
    }

    default_random_engine rng(chrono::system_clock::now().time_since_epoch().count());

    //cout << "shuffling" << endl;
    shuffle(randomPickups.begin(), randomPickups.end(), rng);

    //cout << "randomizing" << endl;
    for(int i = 0; i < randomPickups.size(); i++)
    {
        currRoute.push_back(randomPickups[i]);
        currRoute.push_back(randomPickups[i] + deliveryCopy.size());
    }

    //cout << "done initial randomization" << endl;

    while (temp > END_TEMP && !forceQuit) {
        for (int i = 0; i < MAX_ITER && !forceQuit; i++){
                // cout<<"tests12345"<<endl;
            vector<int> newRoute = neighbourSolution(currRoute, cPI);
                // cout<<"testskjdg"<<endl;
            if(newRoute.size() > 0)
            {
                // cout<<"test"<<endl;
                double newCost = getCheapTravelTime(newRoute, cPI);
                // cout<<"test2"<<endl;
                double delta = newCost - currCost;

                if (delta < 0 || exp(-delta / temp) > (double)rand() / RAND_MAX) {
                    currRoute = newRoute;
                    currCost = newCost;
                    if (currCost < bestCost) {
                        annealingFlag.lock();
                        bestRoute = currRoute;
                        bestCost = currCost;
                        annealingFlag.unlock();
                    }
                }
            }
                // cout<<"test3"<<endl;
        }
        temp *= COOLING_RATE; // Reduce temperature
                // cout<<"test4"<<endl;
    }
    //cout << "ending at temp " << temp << endl;
    forceQuit = true;
    //cout << temp << endl;
    return bestRoute;
}