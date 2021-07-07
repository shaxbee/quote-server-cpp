#include <cstdint>
#include <map>
#include <unordered_map>

#include <boost/range/iterator_range.hpp>

#include "decimal.h"

struct Order {
    enum class Type {
        bid,
        ask
    };

    std::string id;
    Decimal price;
    Decimal size;

    bool operator==(const Order&) const = default;
};

using Bids = std::multimap<Decimal, Order, std::greater<Decimal>>;
using Asks = std::multimap<Decimal, Order, std::less<Decimal>>;

class OrderBook {
public:
    OrderBook(std::int64_t sequence, Bids&& bids, Asks&& asks);
    OrderBook(std::int64_t sequence, std::vector<Order> bids, std::vector<Order> asks);

    template <typename BidsIterT, typename AsksIterT>
    OrderBook(std::int64_t sequence, boost::iterator_range<BidsIterT> bids, boost::iterator_range<AsksIterT> asks);

    inline std::int64_t sequence() { return _sequence; }
    inline const Bids& bids() { return _bids; }
    inline const Asks& asks() { return _asks; }

    void update(std::int64_t sequence, Order::Type type, Order order);
private:
    std::int64_t _sequence;
    std::unordered_map<std::string, Decimal> _prices;
    Bids _bids;
    Asks _asks;

    std::optional<Decimal> remove_price(std::string order_id);
};

template <typename T, typename IterT>
T map_orders(boost::iterator_range<IterT> src) {
    T dst;

    for (auto const& order: src) {
        dst.emplace(order.price, order);
    }

    return dst;
}

template <typename BidsIterT, typename AsksIterT>
OrderBook::OrderBook(std::int64_t sequence, boost::iterator_range<BidsIterT> bids, boost::iterator_range<AsksIterT> asks): OrderBook(sequence, map_orders<Bids, BidsIterT>(bids), map_orders<Asks, AsksIterT>(asks)) {

};
