#include <random>
#include <iostream>
#include <UnitTest++/UnitTest++.h>

#include "StreetsDatabaseAPI.h"
#include "m1.h"
#include "m3.h"
#include "m4.h"

#include "unit_test_util.h"
#include "courier_verify.h"

using ece297test::relative_error;
using ece297test::courier_path_is_legal;


SUITE(extreme_multi_toronto_canada_public) {
    TEST(extreme_multi_toronto_canada) {
        std::vector<DeliveryInf> deliveries;
        std::vector<IntersectionIdx> depots;
        float turn_penalty;
        std::vector<CourierSubPath> result_path;
        bool is_legal;

        deliveries = {DeliveryInf(173551, 129656), DeliveryInf(1580, 152858), DeliveryInf(63729, 2674), DeliveryInf(171755, 176843), DeliveryInf(154269, 95649), DeliveryInf(836, 146883), DeliveryInf(9676, 167636), DeliveryInf(49305, 168006), DeliveryInf(164792, 66641), DeliveryInf(128208, 56289), DeliveryInf(16864, 89540), DeliveryInf(179428, 103697), DeliveryInf(161920, 123888), DeliveryInf(44429, 40730), DeliveryInf(48920, 44865), DeliveryInf(12792, 105184), DeliveryInf(148451, 123201), DeliveryInf(85377, 118211), DeliveryInf(29596, 35577), DeliveryInf(44429, 138352), DeliveryInf(59498, 148293), DeliveryInf(34608, 23268), DeliveryInf(92734, 35577), DeliveryInf(63537, 164451), DeliveryInf(49305, 129739), DeliveryInf(190703, 90052), DeliveryInf(92734, 50511), DeliveryInf(164792, 69536), DeliveryInf(174168, 26098), DeliveryInf(137085, 82778), DeliveryInf(107107, 40730), DeliveryInf(164792, 88515), DeliveryInf(185219, 28853), DeliveryInf(133378, 26098), DeliveryInf(131855, 118416), DeliveryInf(149984, 63637), DeliveryInf(50135, 26098), DeliveryInf(100032, 29567), DeliveryInf(137085, 88470), DeliveryInf(191242, 62513), DeliveryInf(139583, 115613), DeliveryInf(157167, 59428), DeliveryInf(2009, 194312), DeliveryInf(59592, 12185), DeliveryInf(29596, 69104), DeliveryInf(39494, 850), DeliveryInf(135991, 181920), DeliveryInf(161920, 179616), DeliveryInf(38300, 184703), DeliveryInf(122269, 189359), DeliveryInf(105020, 74759), DeliveryInf(75345, 170348), DeliveryInf(142928, 163033), DeliveryInf(45959, 67946), DeliveryInf(120805, 113715), DeliveryInf(170873, 176589), DeliveryInf(139313, 108878), DeliveryInf(164792, 21435), DeliveryInf(127894, 3428), DeliveryInf(29596, 128585), DeliveryInf(156691, 183497), DeliveryInf(83542, 105326), DeliveryInf(137085, 169408), DeliveryInf(137085, 171232), DeliveryInf(90639, 86789), DeliveryInf(143008, 191857), DeliveryInf(139583, 69104), DeliveryInf(63729, 40821), DeliveryInf(171458, 161565), DeliveryInf(148451, 92677), DeliveryInf(148451, 18331), DeliveryInf(104020, 115613), DeliveryInf(59166, 174220), DeliveryInf(146989, 108878), DeliveryInf(114089, 51122), DeliveryInf(109434, 145446), DeliveryInf(119694, 176848), DeliveryInf(59933, 51884), DeliveryInf(117254, 136927), DeliveryInf(179702, 185337), DeliveryInf(49259, 67737), DeliveryInf(85418, 139590), DeliveryInf(47609, 134654), DeliveryInf(69955, 190016), DeliveryInf(54220, 188201), DeliveryInf(4450, 28260), DeliveryInf(172721, 28853), DeliveryInf(59598, 169408), DeliveryInf(92734, 128585), DeliveryInf(184621, 180232), DeliveryInf(118206, 14606), DeliveryInf(49703, 136927), DeliveryInf(166346, 188566), DeliveryInf(145210, 111712), DeliveryInf(139583, 149762), DeliveryInf(137085, 185337), DeliveryInf(59598, 174980), DeliveryInf(89196, 67946), DeliveryInf(29596, 26098), DeliveryInf(17817, 176589), DeliveryInf(153329, 79550), DeliveryInf(99337, 63637), DeliveryInf(59598, 151087), DeliveryInf(63729, 143208), DeliveryInf(17633, 63119), DeliveryInf(97680, 163156), DeliveryInf(113424, 176848), DeliveryInf(69109, 44836), DeliveryInf(28945, 11998), DeliveryInf(92734, 137249), DeliveryInf(161920, 116754), DeliveryInf(137512, 40048), DeliveryInf(48920, 149233), DeliveryInf(148451, 150454), DeliveryInf(64438, 62917), DeliveryInf(92734, 185337), DeliveryInf(63729, 98018), DeliveryInf(169055, 87161), DeliveryInf(124274, 67524), DeliveryInf(137085, 118416), DeliveryInf(131221, 40730), DeliveryInf(49305, 37226), DeliveryInf(164792, 172184), DeliveryInf(44429, 175817), DeliveryInf(35183, 69104), DeliveryInf(99159, 142018), DeliveryInf(92734, 156303), DeliveryInf(92734, 102967), DeliveryInf(12847, 185337), DeliveryInf(97197, 63637), DeliveryInf(90337, 62568), DeliveryInf(122695, 181920), DeliveryInf(29596, 90052), DeliveryInf(126122, 51122), DeliveryInf(169785, 67595), DeliveryInf(64389, 128894), DeliveryInf(121541, 31114), DeliveryInf(49305, 128585), DeliveryInf(20580, 144590), DeliveryInf(139583, 176589), DeliveryInf(99963, 70864), DeliveryInf(88132, 176848), DeliveryInf(137085, 63637), DeliveryInf(176659, 28853), DeliveryInf(147692, 120297), DeliveryInf(59598, 19118), DeliveryInf(46629, 65754), DeliveryInf(129858, 40730), DeliveryInf(108090, 92797), DeliveryInf(76012, 174220), DeliveryInf(59598, 92797), DeliveryInf(59324, 41572), DeliveryInf(131209, 190016), DeliveryInf(139583, 109349), DeliveryInf(164792, 170595), DeliveryInf(46902, 130848), DeliveryInf(75348, 51122), DeliveryInf(51995, 39481), DeliveryInf(131209, 58877), DeliveryInf(29596, 90052), DeliveryInf(47644, 118211), DeliveryInf(92734, 103904), DeliveryInf(108953, 62862), DeliveryInf(63729, 49636), DeliveryInf(49305, 64059), DeliveryInf(63537, 149233), DeliveryInf(132959, 84453), DeliveryInf(85132, 44836), DeliveryInf(63729, 115613), DeliveryInf(44429, 10584), DeliveryInf(63729, 169939), DeliveryInf(131884, 149793), DeliveryInf(95284, 155602), DeliveryInf(44429, 115613), DeliveryInf(69955, 32005), DeliveryInf(69955, 69104), DeliveryInf(137085, 169408), DeliveryInf(164792, 188740), DeliveryInf(165374, 148020), DeliveryInf(92734, 63637), DeliveryInf(108953, 67946), DeliveryInf(112986, 180059), DeliveryInf(174547, 71621), DeliveryInf(125411, 86213), DeliveryInf(131209, 118416), DeliveryInf(188980, 104579), DeliveryInf(77991, 171038), DeliveryInf(406, 44836), DeliveryInf(161920, 130848), DeliveryInf(137275, 29567), DeliveryInf(148603, 105326), DeliveryInf(137350, 52015), DeliveryInf(88603, 116908), DeliveryInf(172361, 78698), DeliveryInf(10470, 107697), DeliveryInf(188231, 16184), DeliveryInf(191059, 130424), DeliveryInf(12869, 48881), DeliveryInf(123606, 91132), DeliveryInf(133378, 53238), DeliveryInf(47451, 57033), DeliveryInf(59598, 118211), DeliveryInf(139583, 108878), DeliveryInf(140154, 190016), DeliveryInf(142841, 92797), DeliveryInf(139583, 112555), DeliveryInf(29596, 28853), DeliveryInf(176131, 130452), DeliveryInf(82057, 129739), DeliveryInf(133378, 57870), DeliveryInf(149984, 190016), DeliveryInf(101691, 127469), DeliveryInf(131209, 32212), DeliveryInf(119964, 185132), DeliveryInf(82157, 148293), DeliveryInf(146695, 190904), DeliveryInf(139693, 69104), DeliveryInf(176659, 159920), DeliveryInf(139583, 205), DeliveryInf(60141, 42685), DeliveryInf(181096, 113104), DeliveryInf(139583, 35577), DeliveryInf(12207, 184703), DeliveryInf(113931, 51122), DeliveryInf(175052, 40730), DeliveryInf(108953, 181920), DeliveryInf(92734, 150433), DeliveryInf(179575, 107526), DeliveryInf(161920, 120179), DeliveryInf(48920, 84745), DeliveryInf(632, 12369), DeliveryInf(29596, 105326), DeliveryInf(153229, 132384), DeliveryInf(108953, 984), DeliveryInf(59598, 92797), DeliveryInf(176659, 594), DeliveryInf(13860, 32756), DeliveryInf(84819, 8671), DeliveryInf(48070, 105326), DeliveryInf(146587, 128585), DeliveryInf(137085, 94500), DeliveryInf(166006, 28853), DeliveryInf(10522, 104385), DeliveryInf(108953, 77380), DeliveryInf(151792, 105326), DeliveryInf(63537, 26098), DeliveryInf(14532, 129739), DeliveryInf(53600, 104031), DeliveryInf(161920, 140498), DeliveryInf(162430, 138000), DeliveryInf(108953, 183504), DeliveryInf(11438, 135242), DeliveryInf(21006, 149088), DeliveryInf(58640, 108878), DeliveryInf(186304, 15120), DeliveryInf(184614, 30596), DeliveryInf(63849, 148190), DeliveryInf(133309, 118211), DeliveryInf(41302, 176589), DeliveryInf(111511, 149233)};
        depots = {65563, 59285, 65060, 98269, 151475};
        turn_penalty = 30.000000000;
        {
        	ECE297_TIME_CONSTRAINT(50000);
        	
        result_path = travelingCourier(turn_penalty, deliveries, depots);
        }

        is_legal = courier_path_is_legal(deliveries, depots, result_path);
        CHECK(is_legal);

        if(is_legal) {
        	double path_cost = ece297test::compute_courier_path_travel_time(result_path, turn_penalty);
        	std::cout << "QoR extreme_multi_toronto_canada: " << path_cost << std::endl;
        } else {
        	std::cout << "QoR extreme_multi_toronto_canada: INVALID" << std::endl;
        }

    } //extreme_multi_toronto_canada

} //extreme_multi_toronto_canada_public

