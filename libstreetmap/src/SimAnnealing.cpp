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
const double START_TEMP = 1000;
const double END_TEMP = 1e-3;
const double COOLING_RATE = 0.99998;
const int MAX_ITER = 10;
int finished = 0;
int threadCount = 30;

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

void exploreIteration(vector<int>& currRoute, double& currCost, double& bestCost, vector<int>& bestRoute, double temp, const CourierProblemInfo cPI, atomic<bool> &forceQuit)
{
    for (int i = 0; i < MAX_ITER && !forceQuit; i++){
        vector<int> newRoute = neighbourSolution(currRoute, cPI);
        // cout<<"testskjdg"<<endl;
        if(newRoute.size() > 0)
        { 
            // cout<<"test"<<endl;
            double newCost = getCheapTravelTime(newRoute, cPI);
            // cout<<"test2"<<endl;
            double delta = newCost - currCost;

            if (delta < 0 || exp(-delta / temp) > (double)rand() / RAND_MAX) {
                annealingFlag.lock();
                currRoute = newRoute;
                currCost = newCost;
                if (currCost < bestCost) {
                    bestRoute = currRoute;
                    bestCost = currCost;
                }
                annealingFlag.unlock();
            }
        }
    }
}

vector<int> simulatedAnnealing(vector<int> &bestRoute
, const CourierProblemInfo& cPI, atomic<bool> &forceQuit, vector<DeliveryInf> deliveryCopy, int explorationThreadCount) {
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
    vector<thread> explorationThreads;
    while (temp > END_TEMP && !forceQuit) {
        limitThreadCount(explorationThreads, explorationThreadCount);
        thread thr(exploreIteration, ref(currRoute), ref(currCost), ref(bestCost), ref(bestRoute), temp, cPI, ref(forceQuit));
        
        explorationThreads.push_back(move(thr));
            // cout<<"test3"<<endl;
        temp *= COOLING_RATE; // Reduce temperature
                // cout<<"test4"<<endl;
    }
    for(int threadIndex = 0; threadIndex < explorationThreads.size(); threadIndex++)
    {
        explorationThreads[threadIndex].join();
    }
    //cout << "ending at temp " << temp << endl;
    //annealingFlag.lock();
    forceQuit = true;
    //cout << temp << endl;
    return bestRoute;
}