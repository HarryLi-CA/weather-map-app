#include "m4.h"
#include "m3.h"
#include <vector>
#include "CourierProblemInfo.h"
#include "2opt.h"
#include "SimAnnealing.h"
#include "thread"
#include <chrono>
#include <atomic>

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;
using namespace std;

std::vector<CourierSubPath> travelingCourier(
    const float turn_penalty,
    const std::vector<DeliveryInf>& deliveries,
    const std::vector<IntersectionIdx>& depots)
{

    auto t1 = high_resolution_clock::now();
    cout << "finding path" << endl;
    CourierProblemInfo cPI{turn_penalty,&deliveries, &depots};
    vector<int> path;
    for(int i = 0; i < deliveries.size(); i++)
    {
        path.push_back(i);
        path.push_back(i + deliveries.size());
    }

    getCheapTravelTime(path, cPI); //cache path travel time
    cout << "switching to simulated annealing" << endl;
    
    double timeLimit = 30;
    setForceQuit(false);
    atomic<bool> isForceQuit = false;

    int maxSAThreads = 30;
    vector<thread> annealingThreads;

    for(int i = 0; i < maxSAThreads; i++)
    {
        thread sA(simulatedAnnealing, ref(path), ref(cPI), ref(isForceQuit), deliveries);
        annealingThreads.push_back(move(sA));
    }
    

    auto t2 = high_resolution_clock::now();
    double elapsedSeconds =  duration_cast<milliseconds>(t2 - t1).count()/1000;
    int lastSeconds = elapsedSeconds + 1;
    while(elapsedSeconds < timeLimit && !isForceQuit)
    {
        t2 = high_resolution_clock::now();
        elapsedSeconds =  duration_cast<milliseconds>(t2 - t1).count()/1000;
        if((int)elapsedSeconds > lastSeconds)
        {
            lastSeconds = elapsedSeconds;
            cout << "Elapsed: " << elapsedSeconds << "s" << endl;
        }
    }
    isForceQuit = true;
    for(int i = 0; i < maxSAThreads; i++)
    {
        annealingThreads[i].join();
    }

    cout << "valid path: " << validPath(path, cPI) << endl;
    
    cout << "switching to 2 opt" << endl;

    timeLimit = 48;
    thread twoOpt(optimize2Opt, ref(path), ref(cPI));

    t2 = high_resolution_clock::now();
    elapsedSeconds =  duration_cast<milliseconds>(t2 - t1).count()/1000;
    lastSeconds = elapsedSeconds + 1;
    while(elapsedSeconds < timeLimit && getDone() == false)
    {
        t2 = high_resolution_clock::now();
        elapsedSeconds =  duration_cast<milliseconds>(t2 - t1).count()/1000;
        if((int)elapsedSeconds > lastSeconds)
        {
            lastSeconds = elapsedSeconds;
            cout << "Elapsed: " << elapsedSeconds << "s" << endl;
        }
    }
    setForceQuit(true);
    twoOpt.join();
    cout << "switching to 3opt" << endl;
    //timeLimit = 48;

    setForceQuit(false);
    setDone(false);

    t2 = high_resolution_clock::now();
    elapsedSeconds =  duration_cast<milliseconds>(t2 - t1).count()/1000;

    if(elapsedSeconds < timeLimit){

    thread threeOpt(optimize3Opt, ref(path), ref(cPI));
        lastSeconds = elapsedSeconds + 1;
        while(elapsedSeconds < timeLimit && getDone() == false)
        {
            t2 = high_resolution_clock::now();
            elapsedSeconds =  duration_cast<milliseconds>(t2 - t1).count()/1000;
            if((int)elapsedSeconds > lastSeconds)
            {
                lastSeconds = elapsedSeconds;
                cout << "Elapsed: " << elapsedSeconds << "s" << endl;
            }
        }
        setForceQuit(true);
        threeOpt.join();
    }
    cout << "joined threads" << endl;
    vector<CourierSubPath> realPath;
    //cout << "n = " << deliveries.size() << endl;
    for(int i = 0; i < path.size() - 1; i++)
    {
        int fromIndex = path[i];
        int toIndex = path[i + 1];
        int n = deliveries.size();
        IntersectionIdx from = (fromIndex < n) ? (deliveries[fromIndex%n].pickUp) : (deliveries[fromIndex%n].dropOff);
        IntersectionIdx to = (toIndex < n) ? (deliveries[toIndex%n].pickUp) : (deliveries[toIndex%n].dropOff);
        //cout << fromIndex << " (" << fromIndex%n << "): " << from << endl;
        if(i == 0)
        {
            IntersectionIdx fromDepot = closestDepot(from, cPI, true).first;
            vector<StreetSegmentIdx> fromPath = cachedPath(fromDepot, from, cPI);
            CourierSubPath fdSP{{fromDepot, from}, fromPath};
            realPath.push_back(fdSP);
            //cout << "added from depot" << endl;
        }
        //cout << from << " -> " << to << endl;
        vector<StreetSegmentIdx> subPath = cachedPath(from, to, cPI);

        CourierSubPath cSP{{from, to}, subPath};

        realPath.push_back(cSP);
        //cout << "added mid-path" << endl;
        if(i == path.size() - 2)
        {
            IntersectionIdx toDepot = closestDepot(to, cPI, false).first;
            vector<StreetSegmentIdx> toPath = cachedPath(to, toDepot, cPI);
            CourierSubPath tdSP{{to, toDepot}, toPath};
            realPath.push_back(tdSP);
            //cout << "added to depot" << endl;
        }
    }

    t2 = high_resolution_clock::now();
    elapsedSeconds =  duration_cast<milliseconds>(t2 - t1).count()/1000;

    cout << "found path at " << elapsedSeconds << "s" << endl;
    return realPath;
}