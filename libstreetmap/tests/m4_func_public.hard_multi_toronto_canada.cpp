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


SUITE(hard_multi_toronto_canada_public) {
    TEST(hard_multi_toronto_canada) {
        std::vector<DeliveryInf> deliveries;
        std::vector<IntersectionIdx> depots;
        float turn_penalty;
        std::vector<CourierSubPath> result_path;
        bool is_legal;

        deliveries = {DeliveryInf(65840, 60953), DeliveryInf(192916, 99534), DeliveryInf(36488, 138914), DeliveryInf(148548, 110367), DeliveryInf(77172, 47396), DeliveryInf(89630, 118327), DeliveryInf(126486, 90236), DeliveryInf(37154, 154609), DeliveryInf(67973, 69887), DeliveryInf(95910, 24968), DeliveryInf(176009, 8147), DeliveryInf(136111, 122118), DeliveryInf(120031, 190483), DeliveryInf(156388, 176440), DeliveryInf(75435, 37921), DeliveryInf(116111, 11725), DeliveryInf(143601, 111890), DeliveryInf(89878, 70968), DeliveryInf(191194, 176440), DeliveryInf(69451, 175514), DeliveryInf(91736, 154609), DeliveryInf(89630, 79802), DeliveryInf(66414, 4048), DeliveryInf(89630, 154151), DeliveryInf(152136, 147909), DeliveryInf(85741, 4866), DeliveryInf(88126, 8462), DeliveryInf(123063, 192084), DeliveryInf(146044, 184578), DeliveryInf(132458, 100808), DeliveryInf(97936, 79805), DeliveryInf(26853, 84051), DeliveryInf(92161, 58286), DeliveryInf(155451, 172102), DeliveryInf(94839, 92373), DeliveryInf(142821, 48233), DeliveryInf(1766, 167198), DeliveryInf(75435, 65559), DeliveryInf(154278, 33456), DeliveryInf(185509, 124535), DeliveryInf(26213, 176368), DeliveryInf(115384, 70036), DeliveryInf(88898, 84606), DeliveryInf(26853, 40586), DeliveryInf(192116, 140295), DeliveryInf(103046, 10973), DeliveryInf(8684, 154609), DeliveryInf(9164, 159614), DeliveryInf(109207, 154609), DeliveryInf(142589, 54022), DeliveryInf(178453, 150376), DeliveryInf(26853, 64787), DeliveryInf(121086, 98940), DeliveryInf(70437, 120154), DeliveryInf(4855, 152936), DeliveryInf(50504, 176088), DeliveryInf(107781, 37966), DeliveryInf(75435, 173157), DeliveryInf(51638, 59557), DeliveryInf(89630, 80888), DeliveryInf(108952, 39894), DeliveryInf(13661, 91105), DeliveryInf(28755, 188685), DeliveryInf(127699, 135015), DeliveryInf(13523, 168569), DeliveryInf(72427, 85304), DeliveryInf(77800, 177734), DeliveryInf(104445, 49692), DeliveryInf(73852, 132722), DeliveryInf(26853, 14119), DeliveryInf(43512, 36548), DeliveryInf(185420, 132666), DeliveryInf(6981, 74227), DeliveryInf(26853, 98107), DeliveryInf(83584, 178359), DeliveryInf(108143, 30243), DeliveryInf(171998, 103633), DeliveryInf(193659, 16084), DeliveryInf(122531, 46088), DeliveryInf(17118, 121966), DeliveryInf(150225, 65168), DeliveryInf(165379, 2186), DeliveryInf(84731, 176440), DeliveryInf(149528, 176440), DeliveryInf(107211, 42540), DeliveryInf(102297, 132740), DeliveryInf(188661, 157754), DeliveryInf(29383, 57698), DeliveryInf(28222, 9650), DeliveryInf(183669, 154609), DeliveryInf(73522, 83377), DeliveryInf(79547, 174075), DeliveryInf(17370, 47366), DeliveryInf(133019, 134143), DeliveryInf(43505, 134749), DeliveryInf(166674, 136427), DeliveryInf(108525, 148962), DeliveryInf(123845, 134401), DeliveryInf(132616, 176440), DeliveryInf(92308, 74041)};
        depots = {26, 99179, 118214};
        turn_penalty = 30.000000000;
        {
        	ECE297_TIME_CONSTRAINT(50000);
        	
        result_path = travelingCourier(turn_penalty, deliveries, depots);
        }

        is_legal = courier_path_is_legal(deliveries, depots, result_path);
        CHECK(is_legal);

        if(is_legal) {
        	double path_cost = ece297test::compute_courier_path_travel_time(result_path, turn_penalty);
        	std::cout << "QoR hard_multi_toronto_canada: " << path_cost << std::endl;
        } else {
        	std::cout << "QoR hard_multi_toronto_canada: INVALID" << std::endl;
        }

    } //hard_multi_toronto_canada

} //hard_multi_toronto_canada_public

