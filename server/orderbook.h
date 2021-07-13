#include <cstdint>
#include <map>
#include <optional>
#include <unordered_map>
#include <shared_mutex>

#include <boost/range/iterator_range.hpp>

#include "decimal.h"

enum class Side {
    bid,
    ask
};

class OrderBook {
public:
    struct Entry {
        std::string order_id;
        Decimal price;
        Decimal size;

        bool operator==(const Entry&) const = default;
    };

    struct Update {
        std::string product_id;
        std::int64_t sequence;
        std::optional<Entry> bid;
        std::optional<Entry> ask;

        constexpr bool empty() const {
            return bid.has_value() && ask.has_value();
        };
    };

    using Bids = std::multimap<Decimal, Entry, std::greater<Decimal>>;
    using Asks = std::multimap<Decimal, Entry, std::less<Decimal>>;

    OrderBook(std::int64_t sequence, Bids&& bids, Asks&& asks);
    OrderBook(std::int64_t sequence, std::vector<Entry> bids, std::vector<Entry> asks);

    template <typename BidsIterT, typename AsksIterT>
    OrderBook(std::int64_t sequence, boost::iterator_range<BidsIterT> bids, boost::iterator_range<AsksIterT> asks);

    inline std::int64_t sequence() const { return _sequence; }
    inline const Bids& bids() const { return _bids; }
    inline const Asks& asks() const { return _asks; }

    // update performs atomic update bids and asks
    // return value contains update orderbook sequence and actual price and size of entry
    //
    // if entry.price == 0 then price is looked up
    // if entry.size > 0 then entry size will be replaced
    // if entry.size < 0 then entry size will be reduced
    Update update(const Update& update);

private:
    std::int64_t _sequence;
    std::unordered_map<std::string, Decimal> _prices;
    Bids _bids;
    Asks _asks;

    template <typename T>
    OrderBook::Entry update(T& entries, const Entry& entry);
};

class OrderBooks {
public:
    explicit OrderBooks(std::unordered_map<std::string, OrderBook>&& data);

    bool get(std::string product_id, std::function<void (const OrderBook&)> callback);
    std::optional<OrderBook::Update> update(const OrderBook::Update& update);

private:
    std::shared_mutex _mtx;
    std::unordered_map<std::string, OrderBook> _data;
};

constexpr Side opposite(Side side) {
    switch (side) {
    case Side::ask:
        return Side::bid;
    case Side::bid:
        return Side::ask;
    };    
};

template <typename T, typename IterT>
T map_entries(boost::iterator_range<IterT> src) {
    T dst;

    for (auto const& entry: src) {
        dst.emplace(entry.price, entry);
    }

    return dst;
}

template <typename BidsIterT, typename AsksIterT>
OrderBook::OrderBook(std::int64_t sequence, boost::iterator_range<BidsIterT> bids, boost::iterator_range<AsksIterT> asks): OrderBook(sequence, map_entries<Bids, BidsIterT>(bids), map_entries<Asks, AsksIterT>(asks)) {

};
