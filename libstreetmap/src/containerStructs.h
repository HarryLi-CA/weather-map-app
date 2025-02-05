#include <vector>
#include <unordered_map>
#include <string.h>
#include "StreetsDatabaseAPI.h"

using namespace std;

//Hash function that allows pairs to be inserted into a unordered_map
struct StreetPairHash {
    size_t operator () (const std::pair<StreetIdx, StreetIdx>& streetPair) const {
    auto hash1 = hash<StreetIdx>{}(streetPair.first);
    auto hash2 = hash<StreetIdx>{}(streetPair.second);
    return hash1 ^ hash2;
    }
};

//Allows pairs to be equated also in any order
struct StreetPairEqual {
    bool operator () (const std::pair<StreetIdx, StreetIdx>& streetPair1, const std::pair<StreetIdx, StreetIdx>& streetPair2) const {
    return (streetPair1.first == streetPair2.first && streetPair1.second == streetPair2.second) || (streetPair1.first == streetPair2.second && streetPair1.second == streetPair2.first);
    }
};

//Compares strings to allow them to be inserted in alphabetical order as well as equated to a partial string
//Important: Capital letters and spaces ignored
struct stringComparator {
    bool operator () (const string& a, const string& b) const {

        int aIndex = 0;
        int bIndex = 0;

        int aLength = a.length();
        int bLength = b.length();
    
        while(aIndex < aLength && bIndex < bLength){
            while(aIndex < aLength && a[aIndex] == ' '){
                aIndex++;
            }
            while(bIndex < bLength && b[bIndex] == ' '){
                bIndex++;
            }

            if(aIndex >= aLength || bIndex >= bLength){
                return false;
            }

            if(toupper(a[aIndex]) < toupper(b[bIndex])){
                return true;
            }
            if(toupper(a[aIndex]) > toupper(b[bIndex])){
                return false;
            }
            aIndex++;
            bIndex++;
        }
        return false;
    }
};