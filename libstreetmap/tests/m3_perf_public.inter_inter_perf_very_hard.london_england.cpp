#include <random>
#include <stdio.h>
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
        rand_lat = std::uniform_real_distribution<double>(51.280006409, 51.699996948);
        rand_lon = std::uniform_real_distribution<double>(-0.599998116, 0.419986486);
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
        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(416, 40765));
        CHECK(path_is_legal(416, 40765, path));
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 3243.00435978340283327);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(23332, 243974));
        CHECK(path_is_legal(23332, 243974, path));
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 4934.38481994501580630);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(29175, 164997));
        CHECK(path_is_legal(29175, 164997, path));
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 1799.96687912114475694);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(37968, 176415));
        CHECK(path_is_legal(37968, 176415, path));
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 2390.14261070152861066);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(101851, 182365));
        CHECK(path_is_legal(101851, 182365, path));
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 1373.72605517595866331);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(111770, 330593));
        CHECK(path_is_legal(111770, 330593, path));
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 1848.67226026048138010);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(119125, 329041));
        CHECK(path_is_legal(119125, 329041, path));
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 3826.75032574111810391);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(133036, 150451));
        CHECK(path_is_legal(133036, 150451, path));
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 2373.12452473315534007);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(163788, 294031));
        CHECK(path_is_legal(163788, 294031, path));
        std::cout << computePathTravelTime(0.00000000000000000, path) << endl;
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 2694.50416567526599465);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(169294, 188312));
        CHECK(path_is_legal(169294, 188312, path));
        std::cout << computePathTravelTime(30.00000000000000000, path) << endl;
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 1838.30537872091076679);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(191776, 188296));
        CHECK(path_is_legal(191776, 188296, path));
        std::cout << computePathTravelTime(0.00000000000000000, path) << endl;
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 2099.65168707628890843);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(82224, 253338));
        CHECK(path_is_legal(82224, 253338, path));
        std::cout << computePathTravelTime(30.00000000000000000, path) << endl;
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 4001.54026260530372383);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(206580, 144650));
        CHECK(path_is_legal(206580, 144650, path));
        std::cout << computePathTravelTime(30.00000000000000000, path) << endl;
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 3333.36904487161154975);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(210690, 226041));
        CHECK(path_is_legal(210690, 226041, path));
        std::cout << computePathTravelTime(30.00000000000000000, path) << endl;
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 1077.55492343421224177);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(227758, 283142));
        CHECK(path_is_legal(227758, 283142, path));
        std::cout << computePathTravelTime(0.00000000000000000, path) << endl;
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 3790.72216052025896715);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(234052, 162035));
        CHECK(path_is_legal(234052, 162035, path));
        std::cout << computePathTravelTime(0.00000000000000000, path) << endl;
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 1388.25807810671267362);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(240485, 20230));
        CHECK(path_is_legal(240485, 20230, path));
        std::cout << computePathTravelTime(0.00000000000000000, path) << endl;
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 1702.61280135703782435);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(250490, 98265));
        CHECK(path_is_legal(250490, 98265, path));
        std::cout << computePathTravelTime(0.00000000000000000, path) << endl;
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 2353.75350048533118752);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(253417, 106719));
        CHECK(path_is_legal(253417, 106719, path));
        std::cout << computePathTravelTime(0.00000000000000000, path) << endl;
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 1893.52189991935779290);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(261541, 255424));
        CHECK(path_is_legal(261541, 255424, path));
        std::cout << computePathTravelTime(30.00000000000000000, path) << endl;
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 3239.97261425242959376);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(58267, 149584));
        CHECK(path_is_legal(58267, 149584, path));
        std::cout << computePathTravelTime(0.00000000000000000, path) << endl;
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 2011.50411892000897751);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(287857, 115591));
        CHECK(path_is_legal(287857, 115591, path));
        std::cout << computePathTravelTime(30.00000000000000000, path) << endl;
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 2935.41815782033017967);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(290693, 324380));
        CHECK(path_is_legal(290693, 324380, path));
        std::cout << computePathTravelTime(30.00000000000000000, path) << endl;
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 3478.42134589164516001);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(281132, 219416));
        CHECK(path_is_legal(281132, 219416, path));
        std::cout << computePathTravelTime(0.00000000000000000, path) << endl;
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 1252.68129830870884689);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(310196, 287714));
        CHECK(path_is_legal(310196, 287714, path));
        std::cout << computePathTravelTime(0.00000000000000000, path) << endl;
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 1614.39641640232548525);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(216649, 152186));
        CHECK(path_is_legal(216649, 152186, path));
        std::cout << computePathTravelTime(30.00000000000000000, path) << endl;
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 3145.55416253628027334);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(43890, 41608));
        CHECK(path_is_legal(43890, 41608, path));
        std::cout << computePathTravelTime(0.00000000000000000, path) << endl;
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 871.50361119254876030);

        path = findPathBetweenIntersections(30.00000000000000000, std::make_pair(156317, 5633));
        CHECK(path_is_legal(156317, 5633, path));
        std::cout << computePathTravelTime(30.00000000000000000, path) << endl;
        CHECK(computePathTravelTime(30.00000000000000000, path) <= 2271.95250290435660645);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(237985, 136352));
        CHECK(path_is_legal(237985, 136352, path));
        std::cout << computePathTravelTime(0.00000000000000000, path) << endl;
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 3032.62667674027079556);

        path = findPathBetweenIntersections(0.00000000000000000, std::make_pair(155252, 2840));
        CHECK(path_is_legal(155252, 2840, path));
        std::cout << computePathTravelTime(0.00000000000000000, path) << endl;
        CHECK(computePathTravelTime(0.00000000000000000, path) <= 1884.89295113595767361);

        //Generate random inputs
        std::vector<IntersectionIdx> intersection_ids1;
        std::vector<IntersectionIdx> intersection_ids2;
        std::vector<double> turn_penalties;
        for(size_t i = 0; i < 100; i++) {
            intersection_ids1.push_back(rand_intersection(rng));
            intersection_ids2.push_back(rand_intersection(rng));
            turn_penalties.push_back(rand_turn_penalty(rng));
        }
        {
            //Timed Test
            ECE297_TIME_CONSTRAINT(9941);
            std::vector<StreetSegmentIdx> result;
            for(size_t i = 0; i < 100; i++) {
                result = findPathBetweenIntersections(turn_penalties[i], std::make_pair(intersection_ids1[i], intersection_ids2[i]));
            }
        }
    } //findPathBetweenIntersections_perf_very_hard

} //inter_inter_path_perf_very_hard_public

