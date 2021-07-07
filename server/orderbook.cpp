#include "orderbook.h"

namespace {
    template <typename T>
    void update_order(T& entries, Order order) {
        auto [begin, end] = entries.equal_range(order.price);
        auto it = std::find_if(begin, end, [&order](const auto& p) {
            return p.second.id == order.id;
        });

        if (it != end && order.size) {
            it->second = order;
        } else if (it != end) {
            entries.erase(it);
        } else if (order.size) {
            entries.emplace(order.price, order);
        }
    }
}

OrderBook::OrderBook(std::int64_t sequence, Bids&& bids, Asks&& asks): _sequence{sequence}, _prices{}, _bids{bids}, _asks{asks} {

};

OrderBook::OrderBook(std::int64_t sequence, std::vector<Order> bids, std::vector<Order> asks): OrderBook(sequence, boost::make_iterator_range(bids), boost::make_iterator_range(asks)) {

};

void OrderBook::update(std::int64_t sequence, Order::Type type, Order order) {
    _sequence = sequence;
    switch (type) {
    case Order::Type::bid:
        update_order(_bids, order);
    case Order::Type::ask:
        update_order(_asks, order);
    };
};