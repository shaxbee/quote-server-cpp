#ifndef COINBASE_ORDERBOOK_H
#define COINBASE_ORDERBOOK_H 1

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

        static Entry FromJSON(const boost::json::object& src);
    };

    std::int64_t sequence;
    std::vector<Entry> bids;
    std::vector<Entry> asks;
};

OrderBook tag_invoke(boost::json::value_to_tag<OrderBook>, boost::json::value const& src);
OrderBook::Entry tag_invoke(boost::json::value_to_tag<OrderBook::Entry>, boost::json::value const& src);

} // namespace coinbase

#endif