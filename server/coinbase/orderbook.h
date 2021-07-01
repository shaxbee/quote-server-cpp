#ifndef COINBASE_ORDERBOOK_H
#define COINBASE_ORDERBOOK_H 1

#include <ostream>
#include <vector>
#include <string>

#include <boost/json.hpp>

#include "decimal.h"

namespace coinbase {

// https://docs.pro.coinbase.com/#get-product-order-book
struct OrderBook {
    struct Entry {
        Decimal price;
        Decimal size;
        std::string order_id;

        bool operator==(const Entry&) const = default;
    };

    std::int64_t sequence;
    std::vector<Entry> bids;
    std::vector<Entry> asks;
};

OrderBook tag_invoke(boost::json::value_to_tag<OrderBook>, boost::json::value const& src) {
    return OrderBook{
        .sequence = src.at("sequence").as_int64(),
        .bids = boost::json::value_to<std::vector<OrderBook::Entry>>(src.at("bids")),
        .asks = boost::json::value_to<std::vector<OrderBook::Entry>>(src.at("asks"))
    };
}

std::ostream& operator<<(std::ostream& os, const OrderBook::Entry& entry) {
    return os << std::setprecision(std::numeric_limits<Decimal>::max_digits10) << "{.price=\"" << entry.price << "\",.size=\"" << entry.size << "\",.order_id=\"" << entry.order_id << "\"}";
}

OrderBook::Entry tag_invoke(boost::json::value_to_tag<OrderBook::Entry>, boost::json::value const& src) {
    auto arr = src.as_array();
    return OrderBook::Entry{
        .price = Decimal{boost::json::value_to<std::string>(arr.at(0))},
        .size = Decimal{boost::json::value_to<std::string>(arr.at(1))},
        .order_id = boost::json::value_to<std::string>(arr.at(2))
    };
}

} // namespace coinbase

#endif