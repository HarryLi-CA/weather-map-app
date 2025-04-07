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


SUITE(hard_multi_london_england_public) {
    TEST(hard_multi_london_england) {
        std::vector<DeliveryInf> deliveries;
        std::vector<IntersectionIdx> depots;
        float turn_penalty;
        std::vector<CourierSubPath> result_path;
        bool is_legal;

        deliveries = {DeliveryInf(284499, 133213), DeliveryInf(210379, 186706), DeliveryInf(240284, 133811), DeliveryInf(311640, 279303), DeliveryInf(24502, 66221), DeliveryInf(93562, 222011), DeliveryInf(270926, 179268), DeliveryInf(173777, 272189), DeliveryInf(127623, 83506), DeliveryInf(323335, 244149), DeliveryInf(31473, 319558), DeliveryInf(144130, 7334), DeliveryInf(91507, 197408), DeliveryInf(53239, 319687), DeliveryInf(195286, 334433), DeliveryInf(197869, 241014), DeliveryInf(269151, 87393), DeliveryInf(194253, 272189), DeliveryInf(139826, 318906), DeliveryInf(166215, 15332), DeliveryInf(346420, 12650), DeliveryInf(336120, 166985), DeliveryInf(281658, 332786), DeliveryInf(47496, 182652), DeliveryInf(125837, 264352), DeliveryInf(31648, 283356), DeliveryInf(224391, 68709), DeliveryInf(131229, 258774), DeliveryInf(341831, 220988), DeliveryInf(335958, 229176), DeliveryInf(171836, 151068), DeliveryInf(260187, 51135), DeliveryInf(52101, 8798), DeliveryInf(162399, 264614), DeliveryInf(167251, 118077), DeliveryInf(3201, 348033), DeliveryInf(348090, 187771), DeliveryInf(31648, 243050), DeliveryInf(120335, 105608), DeliveryInf(284499, 140965), DeliveryInf(275652, 350886), DeliveryInf(159673, 280133), DeliveryInf(162847, 85876), DeliveryInf(217482, 202731), DeliveryInf(153522, 258355), DeliveryInf(31648, 90036), DeliveryInf(177449, 219393), DeliveryInf(284499, 119295), DeliveryInf(214190, 8798), DeliveryInf(136678, 239997), DeliveryInf(301993, 279533), DeliveryInf(203384, 196634), DeliveryInf(67319, 15735), DeliveryInf(209062, 229176), DeliveryInf(195941, 66112), DeliveryInf(284499, 126896), DeliveryInf(231375, 312947), DeliveryInf(299647, 163497), DeliveryInf(222976, 144598), DeliveryInf(151444, 185350), DeliveryInf(31016, 14762), DeliveryInf(189242, 78839), DeliveryInf(246617, 272189), DeliveryInf(24752, 349540), DeliveryInf(155352, 134153)};
        depots = {47, 179700};
        turn_penalty = 30.000000000;
        {
        	ECE297_TIME_CONSTRAINT(50000);
        	
        result_path = travelingCourier(turn_penalty, deliveries, depots);
        }

        is_legal = courier_path_is_legal(deliveries, depots, result_path);
        CHECK(is_legal);

        if(is_legal) {
        	double path_cost = ece297test::compute_courier_path_travel_time(result_path, turn_penalty);
        	std::cout << "QoR hard_multi_london_england: " << path_cost << std::endl;
        } else {
        	std::cout << "QoR hard_multi_london_england: INVALID" << std::endl;
        }

    } //hard_multi_london_england

} //hard_multi_london_england_public

