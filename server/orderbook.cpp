#include "orderbook.h"

#include <iostream>
#include <boost/range/adaptor/map.hpp>

OrderBook::OrderBook(std::int64_t sequence, Bids&& bids, Asks&& asks): _sequence{sequence}, _prices{bids.size() + asks.size()}, _bids{bids}, _asks{asks} {
    for (const auto& entry: bids | boost::adaptors::map_values) {
        _prices.emplace(entry.order_id, entry.price);
    };

    for (const auto& entry: asks | boost::adaptors::map_values) {
        _prices.emplace(entry.order_id, entry.price);
    };
};

OrderBook::OrderBook(std::int64_t sequence, std::vector<Entry> bids, std::vector<Entry> asks): OrderBook(sequence, boost::make_iterator_range(bids), boost::make_iterator_range(asks)) {

};

template <typename T>
OrderBook::Entry OrderBook::update(T& entries, const OrderBook::Entry& entry) {
    auto price = entry.price;
    auto size = entry.size;

    // price is unknown, lookup by order id
    if (price.is_zero()) {
        price = _prices.at(entry.order_id);
    };

    auto [begin, end] = entries.equal_range(price);
    auto it = std::find_if(begin, end, [&entry](const auto& p) {
        return p.second.order_id == entry.order_id;
    });

    // requested update size with delta for order that does not exist
    if (it == end && size.sign() == -1) {
        throw std::invalid_argument("missing entry in update");
    };

    // update size with delta
    if (it != end && size.sign() == -1) {
        size = it->second.size + size;    
    };

    Entry updated{
        .order_id = entry.order_id,
        .price = price,
        .size = size,
    };

    // update exisiting entry
    if (!size.is_zero() && it != end) {
        it->second = updated;
    // insert new entry
    } else if (!size.is_zero() && it == end) {
        entries.emplace(price, updated);
        _prices.emplace(entry.order_id, entry.price);
    // remove entry
    } else if (size.is_zero() && it != end) {
        entries.erase(it);
        _prices.erase(entry.order_id);
    };

    return updated;
};

OrderBook::Update OrderBook::update(const Update& u) {
    std::optional<Entry> bid, ask;

    if (u.bid) {
        bid = update(_bids, u.bid.value());
    };

    if (u.ask) {
        ask = update(_asks, u.ask.value());
    };

    _sequence = u.sequence;

    return Update{
        .product_id = u.product_id,
        .sequence = u.sequence,
        .bid = bid,
        .ask = ask,
    };
};


OrderBooks::OrderBooks(std::unordered_map<std::string, OrderBook>&& data): _data{data} {

};

bool OrderBooks::get(std::string product_id, std::function<void (const OrderBook&)> callback) {
    auto lock = std::shared_lock(_mtx);

    auto it = _data.find(product_id);
    if (it == _data.end()) {
        return false;
    }
    
    callback(it->second);

    return true;
};

std::optional<OrderBook::Update> OrderBooks::update(const OrderBook::Update& update) {
    auto lock = std::unique_lock(_mtx);

    auto& orderbook = _data.at(update.product_id);

    if (update.sequence <= orderbook.sequence()) {
        return std::nullopt;
    };

    if (update.sequence - orderbook.sequence() > 1) {
        throw std::invalid_argument("invalid sequence");
    }

    return orderbook.update(update);
};