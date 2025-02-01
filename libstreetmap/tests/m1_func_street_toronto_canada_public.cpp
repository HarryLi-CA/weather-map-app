
#include "m1.h"
#include "unit_test_util.h"

#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"

#include <UnitTest++/UnitTest++.h>

#include <random>
#include <algorithm>
#include <set>

using ece297test::relative_error;
using ece297test::sorted;

SUITE(street_queries_public_toronto_canada) {

    struct BaseMapFixture {
        BaseMapFixture() {
            //Load the map
            try {
                loadMap("/cad2/ece297s/public/maps/toronto_canada.streets.bin");
            } catch (...) {
                std::cout << "!!!! BaseMapFixture test setup: loadMap threw an exception !!!!" << std::endl;
                throw; // re-throw exception
            }
        }
    
        ~BaseMapFixture() {
            //Clean-up
            try {
                closeMap();
            } catch (const std::exception& e) {
                std::cout << "!!!! BaseMapFixture test teardown: closeMap threw an exception. what(): " << e.what() << " !!!!" << std::endl;
                std::terminate(); // we're in a destructor
            } catch (...) {
                std::cout << "!!!! BaseMapFixture test teardown: closeMap threw an exception !!!!" << std::endl;
                std::terminate(); // we're in a destructor
            }
        }
    };


    struct MapFixture : BaseMapFixture {};

    TEST_FIXTURE(MapFixture, all_street_intersections) {
        std::vector<IntersectionIdx> expected;

        expected = {2, 3, 10, 745, 746, 749, 750, 751, 752, 755, 756, 759, 760, 763, 764, 772, 773, 774, 776, 777, 781, 1917, 1934, 7470, 10465, 10472, 10473, 10597, 10760, 10761, 10768, 10769, 10807, 10808, 14060, 14067, 14072, 15743, 15744, 15745, 15786, 15787, 15904, 15905, 15906, 17904, 17905, 20440, 21452, 25925, 25948, 25949, 31104, 31105, 31106, 31117, 31118, 31119, 31437, 31438, 31439, 31448, 31449, 41767, 41768, 62961, 62962, 63317, 63318, 63331, 80044, 80058, 80059, 80060, 80073, 81003, 81004, 81508, 81509, 81514, 81517, 81518, 81525, 98166, 98415, 98424, 104527, 104528, 121595};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(2)));

        expected = {18866, 18867, 117097, 117098};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(4044)));

        expected = {20392, 20393, 20394, 20397};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(4392)));

        expected = {12182, 24011, 24034};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(5219)));

        expected = {26977, 26978, 26979};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(5774)));

        expected = {28316, 28317, 28318};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(6081)));

        expected = {29890, 29891, 35415, 35416, 35465, 39199, 39201, 39218, 86431, 86432, 86433, 86434, 86435, 86436, 86437, 86438, 86439, 86457, 86458, 86459, 86460, 86461, 86462, 86463, 86464, 86465, 86466, 86467, 86480, 86481, 86482, 86483, 103471, 103472, 103473, 103474, 103475, 113313, 119360, 119362, 119365, 119366, 120226, 131661, 136847, 136848, 142221, 142222, 142223, 142224};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(6296)));

        expected = {52142, 53958};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(9704)));

        expected = {58851, 58873};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(11185)));

        expected = {53287, 62356, 62357, 62358, 62363};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(12113)));

        expected = {63016, 63030};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(12334)));

        expected = {47154, 64964};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(12881)));

        expected = {66025, 66026, 66036};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(12992)));

        expected = {71160, 71161, 71163};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(14258)));

        expected = {75108, 75152};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(15275)));

        expected = {75576, 75599};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(15418)));

        expected = {77033, 77034, 77040, 77042, 77045};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(15810)));

        expected = {33680, 77421, 77437};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(15949)));

        expected = {78715, 78716, 78718, 78719, 78720, 78723, 78731, 78732, 78738, 78740, 103190};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(16309)));

        expected = {80671, 80673, 80674, 80675, 80676, 80677, 80678, 80679, 80683, 80685, 80688, 80693, 80780};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(16901)));

        expected = {87865, 87866};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(17559)));

        expected = {88804, 88805, 88806, 88807, 88808, 88809, 88810};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(17852)));

        expected = {89280, 89694};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(18200)));

        expected = {104834, 104835, 192022};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(19966)));

        expected = {31941, 106425, 106426, 106427, 106428, 106429, 106430};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(20023)));

        expected = {108753, 108754};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(20174)));

        expected = {112225, 112235};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(20367)));

        expected = {99516, 115090, 115092};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(20459)));

        expected = {134336, 142366};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(21707)));

        expected = {21808, 170437, 170439};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfStreet(22301)));

    } //all_street_intersections

    TEST_FIXTURE(MapFixture, intersection_ids_from_street_ids) {
        std::vector<IntersectionIdx> expected;

        expected = {2, 3, 10, 746, 750, 759, 773, 776, 1917, 1934, 7470, 10465, 14060, 14067, 14072, 20440, 21452, 31119, 81518};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(2, 0))));

        expected = {7675, 7676};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(1527, 0))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(4281, 6514))));

        expected = {87881};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(4867, 17568))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(6343, 4530))));

        expected = {36788};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(7165, 16721))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(7364, 2533))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(7712, 13851))));

        expected = {46676};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(8315, 8172))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(8982, 17636))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(10247, 17317))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(11275, 3834))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(11600, 9105))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(12961, 11231))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(13442, 18367))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(15617, 18451))));

        expected = {33680, 77437};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(15949, 0))));

        expected = {78738, 78740};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(16309, 16320))));

        expected = {79395, 79403};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(16550, 16543))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(16683, 506))));

        expected = {80674, 80675, 80677, 80678, 80693, 80780};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(16901, 0))));

        expected = {88805};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(17852, 17888))));

        expected = {89484, 89683};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(18077, 18156))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(19062, 17273))));

        expected = {12297};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(19242, 2594))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(19619, 407))));

        expected = {};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(19820, 22010))));

        expected = {108753};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(20174, 20176))));

        expected = {115090};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(20459, 19373))));

        expected = {127129, 127130, 127131, 127132};
        ECE297_CHECK_EQUAL(expected, sorted(findIntersectionsOfTwoStreets(std::make_pair(20945, 0))));

    } //intersection_ids_from_street_ids

    TEST_FIXTURE(MapFixture, street_ids_from_partial_street_name) {
        std::vector<StreetIdx> expected;

        expected = {15184};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Age")));

        expected = {3626, 8982};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Allard Avenue")));

        expected = {9973, 14342, 16901, 21898};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Beech Street")));

        expected = {19619};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Cedarsprings Way")));

        expected = {1527};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Chapman Road")));

        expected = {15275};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Clipperton Dri")));

        expected = {103, 22571};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Commiss")));

        expected = {12992};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Conamore Cre")));

        expected = {4044};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Dallington Drive")));

        expected = {5219};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Dover Drive")));

        expected = {12961};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Elana")));

        expected = {2533, 4651, 16587};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Elvina ")));

        expected = {17273};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Fishing Crescent")));

        expected = {484, 841, 894, 1143, 1660, 2746, 3386, 3572, 4383, 5003, 5989, 6300, 6858, 7520, 8627, 9876, 14016, 14486, 14515, 15098, 15308, 16091, 16513, 18064, 18305, 18586, 18735, 19251, 22282, 22310};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Ham")));

        expected = {1, 2, 3, 4, 10, 31, 42, 62, 63, 120, 121, 125, 126, 127, 128, 131, 132, 133, 135, 144, 145, 146, 147, 148, 149, 150, 220, 221, 500, 560, 562, 612, 613, 890, 891, 892, 957, 1346, 1352, 1353, 1575, 1596, 1726, 1727, 1728, 1729, 1730, 1946, 1947, 2020, 2033, 2140, 2287, 2317, 2481, 2546, 2861, 2878, 3157, 3275, 3560, 3659, 3718, 4696, 4718, 4882, 5238, 5659, 5795, 5899, 5900, 6242, 6367, 6763, 7528, 8190, 8203, 8312, 9478, 10059, 10264, 10636, 10643, 11034, 11091, 11508, 11593, 11619, 11892, 11916, 12107, 12382, 12424, 12911, 13283, 13404, 13519, 14045, 14855, 15220, 15550, 16219, 16231, 16486, 17092, 18053, 18536, 18616, 19144, 19172, 19699, 20850, 21123, 21262, 21582, 22475};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("High")));

        expected = {19444, 22010};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("John Birchall Road")));

        expected = {6395, 12314};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("King Henrys Boulevard")));

        expected = {8746};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("McRae Road")));

        expected = {15949};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Mimosa Row")));

        expected = {23, 5341, 9951};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Old Yonge S")));

        expected = {4933};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Omni Driv")));

        expected = {1877, 2000, 2440, 3286, 3427, 3504, 4139, 4160, 4307, 4765, 4877, 4915, 5660, 5678, 5827, 5993, 6322, 6492, 7192, 7682, 7891, 8091, 8362, 8466, 8483, 8495, 8696, 9548, 9618, 9998, 10139, 10336, 10434, 10603, 10728, 11312, 11447, 12104, 12213, 12329, 12402, 12913, 13402, 13552, 14428, 14525, 14561, 14886, 14977, 15947, 16014, 16044, 16090, 16103, 16107, 16157, 16245, 16260, 16484, 16525, 16549, 16669, 17042, 17141, 17233, 17554, 17652, 17684, 18312, 18439, 18440, 18574, 19721, 19764, 20925, 21046, 21725, 21856, 22474};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Pin")));

        expected = {652, 16550};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Plains")));

        expected = {7210};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Roseland Gate")));

        expected = {6358};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Rossburn Dr")));

        expected = {18077, 19333};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Royal Coun")));

        expected = {694, 1405, 1759, 1839, 1993, 2321, 2483, 2847, 2994, 3064, 3675, 4015, 4110, 4290, 4460, 4833, 5132, 5507, 6035, 6062, 6063, 6370, 7162, 7286, 7864, 8140, 8264, 8403, 8585, 8652, 8811, 8887, 8897, 9348, 9425, 10021, 10066, 10147, 10177, 10360, 10518, 10541, 10621, 10725, 11009, 11198, 11216, 11245, 11329, 11577, 11742, 12101, 12143, 12334, 12335, 12551, 12625, 12724, 12730, 12745, 12880, 13820, 14293, 14531, 14955, 15304, 15658, 16233, 16466, 16533, 16835, 17166, 17375, 17613, 17710, 17876, 18411, 18413, 18778, 19321, 19926, 20093, 20346, 21097, 21806, 21872};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("San")));

        expected = {4530};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Thursfiel")));

        expected = {12113};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Trailridge Crescent")));

        expected = {6296};
        ECE297_CHECK_EQUAL(expected, sorted(findStreetIdsFromPartialStreetName("Winston Park Drive")));

    } //street_ids_from_partial_street_name

    TEST_FIXTURE(MapFixture, bounding_box_of_street) {
        LatLonBounds actual;
        std::vector<LatLon> expected_vec, actual_vec;

        expected_vec = {LatLon(43.70133972167968750, -79.56082153320312500), LatLon(43.83868789672851562, -79.07078552246093750)};
        actual = findStreetBoundingBox(2);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.77722549438476562, -79.35508728027343750), LatLon(43.77983474731445312, -79.35290527343750000)};
        actual = findStreetBoundingBox(4044);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.77359008789062500, -79.29903411865234375), LatLon(43.77624511718750000, -79.29797363281250000)};
        actual = findStreetBoundingBox(4392);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.59534072875976562, -79.54457855224609375), LatLon(43.59580230712890625, -79.54253387451171875)};
        actual = findStreetBoundingBox(5219);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.79201889038085938, -79.48526763916015625), LatLon(43.79391860961914062, -79.48289489746093750)};
        actual = findStreetBoundingBox(5774);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.61739349365234375, -79.70895385742187500), LatLon(43.61857604980468750, -79.70809936523437500)};
        actual = findStreetBoundingBox(6081);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.50718688964843750, -79.68712615966796875), LatLon(43.51778411865234375, -79.66729736328125000)};
        actual = findStreetBoundingBox(6296);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.86192321777343750, -79.03515625000000000), LatLon(43.86259460449218750, -79.03094482421875000)};
        actual = findStreetBoundingBox(9704);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.81069946289062500, -79.26315307617187500), LatLon(43.81250381469726562, -79.26100158691406250)};
        actual = findStreetBoundingBox(11185);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.78289031982421875, -79.20821380615234375), LatLon(43.78375244140625000, -79.20545196533203125)};
        actual = findStreetBoundingBox(12113);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.82206344604492188, -79.15364074707031250), LatLon(43.82359313964843750, -79.15161132812500000)};
        actual = findStreetBoundingBox(12334);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.76654434204101562, -79.55218505859375000), LatLon(43.76910400390625000, -79.55113983154296875)};
        actual = findStreetBoundingBox(12881);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.75557327270507812, -79.50521850585937500), LatLon(43.75625991821289062, -79.50078582763671875)};
        actual = findStreetBoundingBox(12992);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.74344635009765625, -79.38326263427734375), LatLon(43.74534988403320312, -79.37947082519531250)};
        actual = findStreetBoundingBox(14258);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.55068969726562500, -79.73058319091796875), LatLon(43.55250930786132812, -79.72830963134765625)};
        actual = findStreetBoundingBox(15275);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.60154342651367188, -79.67417144775390625), LatLon(43.60219573974609375, -79.67302703857421875)};
        actual = findStreetBoundingBox(15418);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.55726623535156250, -79.67805480957031250), LatLon(43.55845260620117188, -79.67498779296875000)};
        actual = findStreetBoundingBox(15810);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.57648086547851562, -79.61433410644531250), LatLon(43.57732009887695312, -79.61312866210937500)};
        actual = findStreetBoundingBox(15949);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.62787246704101562, -79.76766967773437500), LatLon(43.63374328613281250, -79.76110839843750000)};
        actual = findStreetBoundingBox(16309);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.69371795654296875, -79.76255035400390625), LatLon(43.69818115234375000, -79.75160980224609375)};
        actual = findStreetBoundingBox(16901);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.83089447021484375, -79.23711395263671875), LatLon(43.83184051513671875, -79.23548889160156250)};
        actual = findStreetBoundingBox(17559);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.84467315673828125, -79.55371093750000000), LatLon(43.84524154663085938, -79.55035400390625000)};
        actual = findStreetBoundingBox(17852);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.78135681152343750, -79.73143768310546875), LatLon(43.78335952758789062, -79.72930145263671875)};
        actual = findStreetBoundingBox(18200);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.83827209472656250, -79.57582855224609375), LatLon(43.83940887451171875, -79.57440948486328125)};
        actual = findStreetBoundingBox(19966);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.72117996215820312, -79.64125823974609375), LatLon(43.72201919555664062, -79.64006805419921875)};
        actual = findStreetBoundingBox(20023);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.80207061767578125, -79.43251800537109375), LatLon(43.80411529541015625, -79.42805480957031250)};
        actual = findStreetBoundingBox(20174);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.86724090576171875, -79.21383666992187500), LatLon(43.86840438842773438, -79.21240997314453125)};
        actual = findStreetBoundingBox(20367);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.88162994384765625, -79.05309295654296875), LatLon(43.88480377197265625, -79.05220031738281250)};
        actual = findStreetBoundingBox(20459);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.80600357055664062, -79.72895812988281250), LatLon(43.80706405639648438, -79.72759246826171875)};
        actual = findStreetBoundingBox(21707);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

        expected_vec = {LatLon(43.88183593750000000, -79.46569061279296875), LatLon(43.88296890258789062, -79.46539306640625000)};
        actual = findStreetBoundingBox(22301);
        actual_vec = {actual.min, actual.max};
        ECE297_CHECK_EQUAL(expected_vec, actual_vec);

    } //bounding_box_of_street

} //street_queries_public_toronto_canada

