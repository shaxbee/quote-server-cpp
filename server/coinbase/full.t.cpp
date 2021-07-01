#include "full.h"

#include <boost/json.hpp>
#include <catch2/catch.hpp>

using namespace boost::json;
using namespace coinbase;

namespace {
    Full parse_full(std::string data) {
        parser p;
        p.write(data);

        return value_to<Full>(p.release());
    }
}

TEST_CASE( "Full is unmarshalled ", "[full]" ) {
    SECTION( "Received (limit)" ) {
        auto data = R"json({"type":"received","time":"2014-11-07T08:19:27.028459Z","product_id":"BTC-USD","sequence":10,"order_id":"d50ec984-77a8-460a-b958-66f114b0de9b","size":"1.34","price":"502.1","side":"buy","order_type":"limit"})json";
        auto full = parse_full(data);
        REQUIRE( full.type == Full::Type::Received );
        REQUIRE( full.as_received() == Full::Received{
            .time = "2014-11-07T08:19:27.028459Z",
            .product_id = "BTC-USD",
            .sequence = 10,
            .order_id = "d50ec984-77a8-460a-b958-66f114b0de9b",
            .order_type = "limit",
            .side = "buy",
            .size = {Decimal{"1.34"}},
            .price = {Decimal{"502.1"}}
        } );
    }

    SECTION( "Received (market)" ) {
        auto data = R"json({"type":"received","time":"2014-11-09T08:19:27.028459Z","product_id":"BTC-USD","sequence":12,"order_id":"dddec984-77a8-460a-b958-66f114b0de9b","funds":"3000.234","side":"buy","order_type":"market" })json";
        auto full = parse_full(data);
        REQUIRE( full.type == Full::Type::Received );
        REQUIRE( full.as_received() == Full::Received{
            .time = "2014-11-09T08:19:27.028459Z",
            .product_id = "BTC-USD",
            .sequence = 12,
            .order_id = "dddec984-77a8-460a-b958-66f114b0de9b",
            .order_type = "market",
            .side = "buy",
            .funds = Decimal("3000.234")
        } );
    }

    SECTION( "Open" ) {
        auto data = R"json({"type":"open","time":"2014-11-07T08:19:27.028459Z","product_id":"BTC-USD","sequence":10,"order_id":"d50ec984-77a8-460a-b958-66f114b0de9b","price":"200.2","remaining_size":"1.00","side":"sell"})json";
        auto full = parse_full(data);
        REQUIRE( full.type == Full::Type::Open );
        REQUIRE( full.as_open() == Full::Open{
            .time = "2014-11-07T08:19:27.028459Z",
            .product_id = "BTC-USD",
            .sequence = 10,
            .order_id = "d50ec984-77a8-460a-b958-66f114b0de9b",
            .price = Decimal{"200.2"},
            .remaining_size = Decimal{"1.00"},
            .side = "sell"
        } );
    }

    SECTION( "Done" ) {
        auto data = R"json({"type":"done","time":"2014-11-07T08:19:27.028459Z","product_id":"BTC-USD","sequence":10,"price":"200.2","order_id":"d50ec984-77a8-460a-b958-66f114b0de9b","reason":"filled","side":"sell","remaining_size":"0"})json";
        auto full = parse_full(data);
        REQUIRE( full.type == Full::Type::Done );
        REQUIRE( full.as_done() == Full::Done{
            .time = "2014-11-07T08:19:27.028459Z",
            .product_id = "BTC-USD",
            .sequence = 10,
            .price = Decimal{"200.2"},
            .order_id = "d50ec984-77a8-460a-b958-66f114b0de9b",
            .reason = "filled",
            .side = "sell"
        } );
    }

    SECTION( "Match" ) {
        auto data = R"json({"type":"match","trade_id":10,"sequence":50,"maker_order_id":"ac928c66-ca53-498f-9c13-a110027a60e8","taker_order_id":"132fb6ae-456b-4654-b4e0-d681ac05cea1","time":"2014-11-07T08:19:27.028459Z","product_id":"BTC-USD","size":"5.23512","price":"400.23","side":"sell"})json";
        auto full = parse_full(data);
        REQUIRE( full.type == Full::Type::Match );
        REQUIRE( full.as_match() == Full::Match{
            .sequence = 50,
            .maker_order_id = "ac928c66-ca53-498f-9c13-a110027a60e8",
            .taker_order_id = "132fb6ae-456b-4654-b4e0-d681ac05cea1",
            .time = "2014-11-07T08:19:27.028459Z",
            .product_id = "BTC-USD",
            .size = Decimal{"5.23512"},
            .price = Decimal{"400.23"},
            .side = "sell"
        } );
    }

    SECTION( "Change" ) {
        auto data = R"json({"type":"change","time":"2014-11-07T08:19:27.028459Z","sequence":80,"order_id":"ac928c66-ca53-498f-9c13-a110027a60e8","product_id":"BTC-USD","new_size":"5.23512","old_size":"12.234412","price":"400.23","side":"sell"})json";
        auto full = parse_full(data);
        REQUIRE( full.type == Full::Type::Change );
        REQUIRE( full.as_change() == Full::Change{
            .time = "2014-11-07T08:19:27.028459Z",
            .sequence = 80,
            .order_id = "ac928c66-ca53-498f-9c13-a110027a60e8",
            .product_id = "BTC-USD",
            .new_size = Decimal{"5.23512"},
            .old_size = Decimal{"12.234412"},
            .price = Decimal{"400.23"},
            .side = "sell"
        } );
    }
}