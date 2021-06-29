#include "orderbook.h"

#include <string>

using namespace boost::json;

namespace coinbase {

OrderBook tag_invoke(value_to_tag<OrderBook>, value const& src) {
    return OrderBook{
        .sequence = src.at("sequence").as_int64(),
        .bids = value_to<std::vector<OrderBook::Entry>>(src.at("bids")),
        .asks = value_to<std::vector<OrderBook::Entry>>(src.at("asks"))
    };
}

OrderBook::Entry tag_invoke(value_to_tag<OrderBook::Entry>, value const& src) {
    return OrderBook::Entry{
        .price = Decimal{value_to<std::string>(src.at("price"))},
        .size = Decimal{value_to<std::string>(src.at("size"))},
        .order_id = value_to<std::string>(src.at("order_id"))
    };
}

} // namespace coinbase