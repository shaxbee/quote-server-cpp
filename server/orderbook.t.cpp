#include "orderbook.h"

#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/equal.hpp>

#include <catch2/catch.hpp>

TEST_CASE( "OrderBook update", "[orderbook]" ) {
    std::vector<Order> bids{
        {.id = "de43f91d-8db9-486e-868c-8389d2611ab0", .price = Decimal{"2.0"}, .size = Decimal{"1.0"}},
        {.id = "77c7c96d-f171-4695-831f-de3c8f6ed2d7", .price = Decimal{"1.0"}, .size = Decimal{"1.0"}},
        {.id = "dd3c42ec-2fcd-4069-881b-667d64714e79", .price = Decimal{"1.0"}, .size = Decimal{"1.0"}},
    };

    std::vector<Order> asks{
        {.id = "e68b5cb3-5d97-4085-9078-1d95995ad8ce", .price = Decimal{"2.1"}, .size = Decimal{"1.0"}},
        {.id = "b37c144f-ad9c-4490-9228-b80766829dcc", .price = Decimal{"2.1"}, .size = Decimal{"1.0"}},
        {.id = "09d86b54-7e8f-46d0-b425-151f24914c36", .price = Decimal{"3.0"}, .size = Decimal{"1.0"}},
    };

    OrderBook orderbook{0, bids, asks};

    // check if asks retain order
    REQUIRE( boost::range::equal(boost::adaptors::values(orderbook.asks()), std::vector<Order>{
        {.id = "e68b5cb3-5d97-4085-9078-1d95995ad8ce", .price = Decimal{"2.1"}, .size = Decimal{"1.0"}},
        {.id = "b37c144f-ad9c-4490-9228-b80766829dcc", .price = Decimal{"2.1"}, .size = Decimal{"1.0"}},
        {.id = "09d86b54-7e8f-46d0-b425-151f24914c36", .price = Decimal{"3.0"}, .size = Decimal{"1.0"}},
    }) );

    // update size from 1.0 to 0.5
    orderbook.update(1, Order::Type::bid, Order{
       .id ="77c7c96d-f171-4695-831f-de3c8f6ed2d7", .price = Decimal{"1.0"}, .size = Decimal{"0.5"} 
    });

    REQUIRE( boost::range::equal(boost::adaptors::values(orderbook.bids()), std::vector<Order>{
        {.id = "de43f91d-8db9-486e-868c-8389d2611ab0", .price = Decimal{"2.0"}, .size = Decimal{"1.0"}},
        {.id = "77c7c96d-f171-4695-831f-de3c8f6ed2d7", .price = Decimal{"1.0"}, .size = Decimal{"0.5"}},
        {.id ="dd3c42ec-2fcd-4069-881b-667d64714e79", .price = Decimal{"1.0"}, .size = Decimal{"1.0"}},
    }) );

    // remove order
    orderbook.update(1, Order::Type::bid, Order{
        .id = "77c7c96d-f171-4695-831f-de3c8f6ed2d7", .price = Decimal{"1.0"}, .size = Decimal{"0.0"}
    });

    REQUIRE( boost::range::equal(boost::adaptors::values(orderbook.bids()), std::vector<Order>{
        {.id = "de43f91d-8db9-486e-868c-8389d2611ab0", .price = Decimal{"2.0"}, .size = Decimal{"1.0"}},
        {.id = "dd3c42ec-2fcd-4069-881b-667d64714e79", .price = Decimal{"1.0"}, .size = Decimal{"1.0"}},
    }) );
}