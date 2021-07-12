#include "orderbook.h"

#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/equal.hpp>

#include <catch2/catch.hpp>

namespace {
    template <typename T>
    bool entries_equal(const T& entries, std::vector<OrderBook::Entry> expected) {
        return boost::range::equal(boost::adaptors::values(entries), expected);
    };
} // anonymous namespace

TEST_CASE( "OrderBook update", "[orderbook]" ) {
    std::vector<OrderBook::Entry> bids{
        {.order_id = "de43f91d-8db9-486e-868c-8389d2611ab0", .price = Decimal{"2.0"}, .size = Decimal{"1.0"}},
        {.order_id = "77c7c96d-f171-4695-831f-de3c8f6ed2d7", .price = Decimal{"1.0"}, .size = Decimal{"1.0"}},
        {.order_id = "dd3c42ec-2fcd-4069-881b-667d64714e79", .price = Decimal{"1.0"}, .size = Decimal{"1.0"}},
    };

    std::vector<OrderBook::Entry> asks{
        {.order_id = "e68b5cb3-5d97-4085-9078-1d95995ad8ce", .price = Decimal{"2.1"}, .size = Decimal{"1.0"}},
        {.order_id = "b37c144f-ad9c-4490-9228-b80766829dcc", .price = Decimal{"2.1"}, .size = Decimal{"1.0"}},
        {.order_id = "09d86b54-7e8f-46d0-b425-151f24914c36", .price = Decimal{"3.0"}, .size = Decimal{"1.0"}},
    };

    OrderBook orderbook{0, bids, asks};

    // check if bids and asks retained order
    REQUIRE( entries_equal(orderbook.bids(), bids) );
    REQUIRE( entries_equal(orderbook.asks(), asks) );

    // update size from 1.0 to 0.5
    auto u1 = orderbook.update({
        .bid{{.order_id ="de43f91d-8db9-486e-868c-8389d2611ab0", .price = Decimal{"2.0"}, .size = Decimal{"0.5"}}},
    });

    REQUIRE( u1.sequence == 1 );
    REQUIRE( orderbook.sequence() == 1 );
    REQUIRE( entries_equal(orderbook.bids(), {
        {.order_id = "de43f91d-8db9-486e-868c-8389d2611ab0", .price = Decimal{"2.0"}, .size = Decimal{"0.5"}},
        {.order_id = "77c7c96d-f171-4695-831f-de3c8f6ed2d7", .price = Decimal{"1.0"}, .size = Decimal{"1.0"}},
        {.order_id ="dd3c42ec-2fcd-4069-881b-667d64714e79", .price = Decimal{"1.0"}, .size = Decimal{"1.0"}},
    }) );

    // remove order
    auto u2 = orderbook.update({
        .bid{{.order_id = "77c7c96d-f171-4695-831f-de3c8f6ed2d7", .price = Decimal{"1.0"}, .size = Decimal{"0.0"}}},
    });

    REQUIRE( u2.sequence == 2 );
    REQUIRE( orderbook.sequence() == 2 );
    REQUIRE( entries_equal(orderbook.bids(), {
        {.order_id = "de43f91d-8db9-486e-868c-8389d2611ab0", .price = Decimal{"2.0"}, .size = Decimal{"0.5"}},
        {.order_id = "dd3c42ec-2fcd-4069-881b-667d64714e79", .price = Decimal{"1.0"}, .size = Decimal{"1.0"}},
    }) );

    // update size with delta
    auto u3 = orderbook.update({
        .ask{{.order_id = "e68b5cb3-5d97-4085-9078-1d95995ad8ce", .size = Decimal{"-0.5"} }},
    });

    REQUIRE( u3.sequence == 3 );
    REQUIRE( orderbook.sequence() == 3 );
    REQUIRE( entries_equal(orderbook.asks(), {
        {.order_id = "e68b5cb3-5d97-4085-9078-1d95995ad8ce", .price = Decimal{"2.1"}, .size = Decimal{"0.5"}},
        {.order_id = "b37c144f-ad9c-4490-9228-b80766829dcc", .price = Decimal{"2.1"}, .size = Decimal{"1.0"}},
        {.order_id = "09d86b54-7e8f-46d0-b425-151f24914c36", .price = Decimal{"3.0"}, .size = Decimal{"1.0"}},
    }) );

    // update both bids and asks
    auto u4 = orderbook.update({
        .bid{{.order_id = "de43f91d-8db9-486e-868c-8389d2611ab0", .size = Decimal{"-0.5"}}},
        .ask{{.order_id = "e68b5cb3-5d97-4085-9078-1d95995ad8ce", .size = Decimal{"-0.5"}}},
    });

    REQUIRE( u4.sequence == 4 );
    REQUIRE( orderbook.sequence() == 4 );
    REQUIRE( entries_equal(orderbook.bids(), {
        {.order_id = "dd3c42ec-2fcd-4069-881b-667d64714e79", .price = Decimal{"1.0"}, .size = Decimal{"1.0"}},
    }) );
    REQUIRE( entries_equal(orderbook.asks(), {
        {.order_id = "b37c144f-ad9c-4490-9228-b80766829dcc", .price = Decimal{"2.1"}, .size = Decimal{"1.0"}},
        {.order_id = "09d86b54-7e8f-46d0-b425-151f24914c36", .price = Decimal{"3.0"}, .size = Decimal{"1.0"}},
    }) );
}