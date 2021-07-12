#ifndef COINBASE_ORDERBOOK_H
#define COINBASE_ORDERBOOK_H 1

#include <ostream>
#include <vector>
#include <string>

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

    bool operator==(const OrderBook&) const = default;
};

OrderBook parse_orderbook(std::string data);

std::ostream& operator<<(std::ostream&, const OrderBook&);
std::ostream& operator<<(std::ostream&, const OrderBook::Entry&);

} // namespace coinbase

#endif