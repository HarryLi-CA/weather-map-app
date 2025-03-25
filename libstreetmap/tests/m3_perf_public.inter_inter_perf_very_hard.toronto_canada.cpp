#include <random>
#include <UnitTest++/UnitTest++.h>

#include "StreetsDatabaseAPI.h"
#include "m1.h"
#include "m3.h"

#include "unit_test_util.h"
#include "path_verify.h"

using ece297test::relative_error;
using ece297test::path_is_legal;


SUITE(inter_inter_path_perf_very_hard_public) {
struct MapFixture {
    MapFixture() {
        rng = std::minstd_rand(4);
        rand_intersection = std::uniform_int_distribution<IntersectionIdx>(0, getNumIntersections()-1);
        rand_street = std::uniform_int_distribution<StreetIdx>(1, getNumStreets()-1);
        rand_segment = std::uniform_int_distribution<StreetSegmentIdx>(0, getNumStreetSegments()-1);
        rand_poi = std::uniform_int_distribution<POIIdx>(0, getNumPointsOfInterest()-1);
        rand_lat = std::uniform_real_distribution<double>(43.479999542, 43.919982910);
        rand_lon = std::uniform_real_distribution<double>(-79.789985657, -79.000000000);
        rand_turn_penalty = std::uniform_real_distribution<double>(0., 30.);
        rand_walking_speed = std::uniform_real_distribution<double>(0.8, 5);
        rand_walking_time_limit = std::uniform_real_distribution<double>(0.0, 300);
    }

    std::minstd_rand rng;
    std::uniform_int_distribution<IntersectionIdx> rand_intersection;
    std::uniform_int_distribution<StreetSegmentIdx> rand_street;
    std::uniform_int_distribution<StreetSegmentIdx> rand_segment;
    std::uniform_int_distribution<POIIdx> rand_poi;
    std::uniform_real_distribution<double> rand_lat;
    std::uniform_real_distribution<double> rand_lon;
    std::uniform_real_distribution<double> rand_turn_penalty;
    std::uniform_real_distribution<double> rand_walking_speed;
    std::uniform_real_distribution<double> rand_walking_time_limit;
};
    TEST_FIXTURE(MapFixture, findPathBetweenIntersections_perf_very_hard) {
        //Verify Functionality
        std::vector<StreetSegmentIdx> path;
        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(6930, 159000));
        CHECK(path_is_legal(6930, 159000, path));
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 1961.62864052091936173);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(7266, 53553));
        CHECK(path_is_legal(7266, 53553, path));
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 1977.84182214424777158);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(24629, 178412));
        CHECK(path_is_legal(24629, 178412, path));
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 1301.99427890515812578);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(29966, 31924));
        CHECK(path_is_legal(29966, 31924, path));
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 2382.08537704006766944);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(31864, 77774));
        CHECK(path_is_legal(31864, 77774, path));
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 1377.68526568237120955);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(37674, 192727));
        CHECK(path_is_legal(37674, 192727, path));
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 1423.84798173702392887);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(56965, 72605));
        CHECK(path_is_legal(56965, 72605, path));
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 1256.40345470425222629);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(60770, 50016));
        CHECK(path_is_legal(60770, 50016, path));
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 1775.40521851320340829);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(62705, 142517));
        CHECK(path_is_legal(62705, 142517, path));
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 2142.87658478941330031);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(66654, 63947));
        CHECK(path_is_legal(66654, 63947, path));
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 1043.12509764721858119);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(69791, 57782));
        CHECK(path_is_legal(69791, 57782, path));
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 856.97141074099295110);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(74301, 23822));
        CHECK(path_is_legal(74301, 23822, path));
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 1355.59446314037199954);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(56712, 94490));
        CHECK(path_is_legal(56712, 94490, path));
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 1184.76757159511225836);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(83173, 8816));
        CHECK(path_is_legal(83173, 8816, path));
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 991.93683825221762618);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(87526, 180200));
        CHECK(path_is_legal(87526, 180200, path));
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 1752.41557268318979368);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(5235, 186912));
        CHECK(path_is_legal(5235, 186912, path));
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 2013.14904176662662394);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(119412, 153085));
        CHECK(path_is_legal(119412, 153085, path));
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 1251.32183945743213371);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(73293, 161301));
        CHECK(path_is_legal(73293, 161301, path));
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 2748.07528042764261045);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(126859, 5719));
        CHECK(path_is_legal(126859, 5719, path));
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 2435.13156175708991213);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(132175, 174762));
        CHECK(path_is_legal(132175, 174762, path));
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 428.87185316232483956);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(29799, 112932));
        CHECK(path_is_legal(29799, 112932, path));
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 2127.59716377229597128);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(147179, 121232));
        CHECK(path_is_legal(147179, 121232, path));
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 1284.98308914347740028);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(157044, 75896));
        CHECK(path_is_legal(157044, 75896, path));
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 2840.40054579276920776);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(157179, 183378));
        CHECK(path_is_legal(157179, 183378, path));
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 1568.36098209378496904);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(163125, 63061));
        CHECK(path_is_legal(163125, 63061, path));
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 609.46144478052076465);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(165740, 126994));
        CHECK(path_is_legal(165740, 126994, path));
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 1459.09179478634519000);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(128966, 20948));
        CHECK(path_is_legal(128966, 20948, path));
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 2631.98107452962040043);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(170692, 44598));
        CHECK(path_is_legal(170692, 44598, path));
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 3019.80746947688976434);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(182926, 37672));
        CHECK(path_is_legal(182926, 37672, path));
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 839.58366987349950250);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(189505, 139886));
        CHECK(path_is_legal(189505, 139886, path));
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 2499.39426472257810019);

        //Generate random inputs
        std::vector<IntersectionIdx> intersection_ids1;
        std::vector<IntersectionIdx> intersection_ids2;
        std::vector<double> turn_penalties;
        for(size_t i = 0; i < 475; i++) {
            intersection_ids1.push_back(rand_intersection(rng));
            intersection_ids2.push_back(rand_intersection(rng));
            turn_penalties.push_back(rand_turn_penalty(rng));
        }
        {
            //Timed Test
            ECE297_TIME_CONSTRAINT(16391);
            std::vector<StreetSegmentIdx> result;
            for(size_t i = 0; i < 475; i++) {
                result = findPathBetweenIntersections(turn_penalties[i], std::make_pair(intersection_ids1[i], intersection_ids2[i]));
            }
        }
    } //findPathBetweenIntersections_perf_very_hard

} //inter_inter_path_perf_very_hard_public

