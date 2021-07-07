#include "orderbook.h"

#include <boost/json.hpp>

namespace coinbase {

OrderBook parse_orderbook(std::string data) {
    boost::json::parser p;
    p.write(data);

    return value_to<OrderBook>(p.release());  
};

OrderBook tag_invoke(boost::json::value_to_tag<OrderBook>, boost::json::value const& src) {
    return OrderBook{
        .sequence = src.at("sequence").as_int64(),
        .bids = boost::json::value_to<std::vector<OrderBook::Entry>>(src.at("bids")),
        .asks = boost::json::value_to<std::vector<OrderBook::Entry>>(src.at("asks"))
    };
};

OrderBook::Entry tag_invoke(boost::json::value_to_tag<OrderBook::Entry>, boost::json::value const& src) {
    auto arr = src.as_array();
    return OrderBook::Entry{
        .price = Decimal{boost::json::value_to<std::string>(arr.at(0))},
        .size = Decimal{boost::json::value_to<std::string>(arr.at(1))},
        .order_id = boost::json::value_to<std::string>(arr.at(2))
    };
};

std::ostream& operator<<(std::ostream& os, const OrderBook& v) {
    os << "{.sequence=" << v.sequence;

    if (!v.bids.empty()) {
        os << ", .bids={";
        std::copy(v.bids.begin(), v.bids.end(), std::ostream_iterator<const OrderBook::Entry&>(os, ", "));
        os << "}";
    }

    if (!v.asks.empty()) {
        os << ", asks={";
        std::copy(v.asks.begin(), v.asks.end(), std::ostream_iterator<const OrderBook::Entry&>(os, ", "));
    }

    return os << "}";
}

std::ostream& operator<<(std::ostream& os, const OrderBook::Entry& v) {
    return os << "{.price=" << v.price << ", .size=" << v.size << ", .order_id=" << v.order_id << "}";
}

} // namespace coinbase