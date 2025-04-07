#include "m4.h"
#include "m3.h"
#include <vector>
#include "CourierProblemInfo.h"
#include "2opt.h"
#include "thread"
#include <chrono>

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
    cout << "finding path" << endl;
    CourierProblemInfo cPI{turn_penalty,&deliveries, &depots};
    vector<int> path;
    for(int i = 0; i < deliveries.size(); i++)
    {
        path.push_back(i);
        path.push_back(i + deliveries.size());
    }

    auto t1 = high_resolution_clock::now();
    double timeLimit = 30;
    setForceQuit(false);
    thread threeOpt(optimize2Opt, ref(path), ref(cPI));

    auto t2 = high_resolution_clock::now();
    double elapsedSeconds =  duration_cast<milliseconds>(t2 - t1).count()/1000;
    int lastSeconds = elapsedSeconds + 1;
    while(elapsedSeconds < timeLimit && getDone() == false)
    {
        t2 = high_resolution_clock::now();
        elapsedSeconds =  duration_cast<milliseconds>(t2 - t1).count()/1000;
        if((int)elapsedSeconds > lastSeconds)
        {
            lastSeconds = elapsedSeconds;
            //cout << "Elapsed: " << elapsedSeconds << "s" << endl;
        }
    }
    setForceQuit(true);
    cout << "switching to 2opt" << endl;
    threeOpt.join();
    timeLimit = 45;
    setForceQuit(false);
    thread twoOpt(optimize3Opt, ref(path), ref(cPI));

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
            //cout << "Elapsed: " << elapsedSeconds << "s" << endl;
        }
    }
    setForceQuit(true);
    twoOpt.join();
    //cout << "joined threads" << endl;

    vector<CourierSubPath> realPath;
    //cout << "n = " << deliveries.size() << endl;
    for(int i = 0; i < path.size() - 1; i++)
    {
        int fromIndex = path[i];
        int toIndex = path[i + 1];
        int n = deliveries.size();
        IntersectionIdx from = (fromIndex < n) ? (deliveries[fromIndex%n].pickUp) : (deliveries[fromIndex%n].dropOff);
        IntersectionIdx to = (toIndex < n) ? (deliveries[toIndex%n].pickUp) : (deliveries[toIndex%n].dropOff);
        //cout << fromIndex << ": " << from << endl;
        if(i == 0)
        {
            IntersectionIdx fromDepot = closestDepot(from, cPI, true).first;
            vector<StreetSegmentIdx> fromPath = findPathBetweenIntersections(cPI.turn_penalty, {fromDepot, from});
            CourierSubPath fdSP{{fromDepot, from}, fromPath};
            realPath.push_back(fdSP);
            //cout << "added from depot" << endl;
        }
        //cout << from << " -> " << to << endl;
        vector<StreetSegmentIdx> subPath = findPathBetweenIntersections(cPI.turn_penalty, {from, to});

        CourierSubPath cSP{{from, to}, subPath};

        realPath.push_back(cSP);
        //cout << "added mid-path" << endl;
        if(i == path.size() - 2)
        {
            IntersectionIdx toDepot = closestDepot(to, cPI, false).first;
            vector<StreetSegmentIdx> toPath = findPathBetweenIntersections(cPI.turn_penalty, {to, toDepot});
            CourierSubPath tdSP{{to, toDepot}, toPath};
            realPath.push_back(tdSP);
            //cout << "added to depot" << endl;
        }
    }
    cout << "found path" << endl;
    return realPath;
}