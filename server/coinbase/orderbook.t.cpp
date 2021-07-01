#include "orderbook.h"

#include <boost/json.hpp>
#include <catch2/catch.hpp>

using namespace boost::json;
using namespace coinbase;

TEST_CASE( "OrderBook is unmarshaled", "[orderbook]" ) {
    auto data = R"json({"bids":[["36206.76","3009.99944762","92f09c53-5dc0-4985-9365-dc628b9d492c"],["36206.75","4771","6640aa13-c0e4-4a71-9848-3477c06d280d"],["36206.74","6020","b1143b94-1995-4ede-aab3-3325d69b243d"],["36206.73","6989","8553ae5b-f724-486a-9331-6c050c5e26c4"],["36206.72","7781","3922a498-e2dc-45e7-b382-3c056326832c"]],"asks":[["36206.78","3009.99972423","20ab9c48-171e-4eea-bb22-5c486712a4b9"],["36206.79","4771","237570e5-caf9-4cf7-8add-1b298e71928d"],["36206.8","6020","3263aefa-3b62-4e8b-8c4a-90679a065019"],["36206.81","6989","d25c3137-0b98-46a1-9753-1ad04873c064"],["36206.82","7781","c3f6b5c5-9e5e-4344-ae15-074a93044bf0"]],"sequence":352174631})json";

    parser p;
    p.write(data);

    auto jv = p.release();
    auto orderbook = value_to<OrderBook>(jv);

    REQUIRE( orderbook.sequence == 352174631 );
    REQUIRE( orderbook.bids == std::vector<OrderBook::Entry>{ 
        { .price = Decimal{"36206.76"}, .size = Decimal{"3009.99944762"}, .order_id = "92f09c53-5dc0-4985-9365-dc628b9d492c"},
        { .price = Decimal{"36206.75"}, .size = Decimal{"4771"}, .order_id = "6640aa13-c0e4-4a71-9848-3477c06d280d"},
        { .price = Decimal{"36206.74"}, .size = Decimal{"6020"}, .order_id = "b1143b94-1995-4ede-aab3-3325d69b243d"},
        { .price = Decimal{"36206.73"}, .size = Decimal{"6989"}, .order_id = "8553ae5b-f724-486a-9331-6c050c5e26c4"},
        { .price = Decimal{"36206.72"}, .size = Decimal{"7781"}, .order_id = "3922a498-e2dc-45e7-b382-3c056326832c"}
    } );
    REQUIRE( orderbook.asks == std::vector<OrderBook::Entry>{
        {.price = Decimal{"36206.78"}, .size= Decimal{"3009.99972423"}, .order_id="20ab9c48-171e-4eea-bb22-5c486712a4b9"},
        {.price = Decimal{"36206.79"}, .size= Decimal{"4771"}, .order_id="237570e5-caf9-4cf7-8add-1b298e71928d"},
        {.price = Decimal{"36206.8"}, .size= Decimal{"6020"}, .order_id= "3263aefa-3b62-4e8b-8c4a-90679a065019"},
        {.price = Decimal{"36206.81"}, .size= Decimal{"6989"}, .order_id="d25c3137-0b98-46a1-9753-1ad04873c064"}, 
        {.price = Decimal{"36206.82"}, .size= Decimal{"7781"}, .order_id="c3f6b5c5-9e5e-4344-ae15-074a93044bf0"}
    } );
}

