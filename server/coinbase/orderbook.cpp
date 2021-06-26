#include "orderbook.h"

#include <string>

using namespace boost::json;

namespace coinbase {

OrderBook tag_invoke(value_to_tag<OrderBook>, value const& src) {
    return OrderBook{
        std::stoi(value_to<std::string>(src.at("sequence"))),
        value_to<std::vector<OrderBook::Entry>>(src.at("bids")),
        value_to<std::vector<OrderBook::Entry>>(src.at("asks"))
    };
}

OrderBook::Entry tag_invoke(value_to_tag<OrderBook::Entry>, value const& src) {
    return OrderBook::Entry{
        decimal{value_to<std::string>(src.at("price"))},
        decimal{value_to<std::string>(src.at("size"))},
        value_to<std::string>(src.at("order_id"))
    };
}

} // namespace coinbase