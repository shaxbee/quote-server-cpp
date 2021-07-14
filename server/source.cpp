#include "source.h"

#include <boost/log/common.hpp>
#include <boost/range/adaptors.hpp>

#include "task_group.h"

namespace {

Side map_side(const std::string& src) {
    if (src == "buy") {
        return Side::bid;
    } else if (src == "sell") {
        return Side::ask;
    } else {
        throw std::invalid_argument("invalid side");
    }
}

OrderBook::Entry map_orderbook_entry(const coinbase::OrderBook::Entry& src) {
    return OrderBook::Entry{
        .order_id = src.order_id,
        .price = src.price,
        .size = src.size,
    };
};

OrderBook map_orderbook(const coinbase::OrderBook& src) {
    auto bids = src.bids | boost::adaptors::transformed(map_orderbook_entry);
    auto asks = src.asks | boost::adaptors::transformed(map_orderbook_entry);

    return OrderBook(src.sequence, bids, asks);
};

} // anonymous namespace

FullVisitor::FullVisitor(): _orderbook_buffer{}, _trade_buffer{} {

};

void FullVisitor::visit(const coinbase::Full& full, const coinbase::Received& received) {
    // bump sequence
    push_orderbook_update(full, {});
}

void FullVisitor::visit(const coinbase::Full& full, const coinbase::Open& open) {
    push_orderbook_entry(full, {
        .order_id = open.order_id,
        .price = open.price,
        .size = open.remaining_size,
    });
};

void FullVisitor::visit(const coinbase::Full& full, const coinbase::Done& done) {
    // market orders are never on orderbook and have price 0
    if (!done.price) {
        // bump sequence
        push_orderbook_update(full,{});
        return;
    };

    push_orderbook_entry(full, {
        .order_id = done.order_id,
        .price = done.price,
    });
};

void FullVisitor::visit(const coinbase::Full& full, const coinbase::Match& match) {
    push_orderbook_entry(full, {
        .order_id = match.maker_order_id,
        .size = -match.size,
    });

    _trade_buffer.emplace_back({
        .product_id = full.product_id,
        .time = full.time,
        .side = map_side(full.side),
        .maker_order_id = match.maker_order_id,
        .taker_order_id = match.taker_order_id,
        .price = match.price,
        .size = match.size,
    });
};

void FullVisitor::visit(const coinbase::Full& full, const coinbase::Change& change) {
    // market orders are never on orderbook and have price 0
    if (!change.price) {
        // bump sequence
        push_orderbook_update(full, {});
        return;
    };

    push_orderbook_entry(full, {
        .order_id = change.order_id,
        .size = change.new_size - change.old_size,
    });
}

OrderBook::Update FullVisitor::pop_orderbook() {
    return _orderbook_buffer.pop_front();
};

Trade FullVisitor::pop_trade() {
    return _trade_buffer.pop_front();
};

void FullVisitor::discard_until(std::int64_t sequence) {
    _orderbook_buffer.discard_until(sequence, [](auto sequence, const auto& update) { 
        return sequence < update.sequence; 
    });
};

void FullVisitor::push_orderbook_update(const coinbase::Full& full, OrderBook::Update&& update) {
    _orderbook_buffer.emplace_back({
        .product_id = full.product_id,
        .sequence = full.sequence,
        .time = full.time,
        .bid = update.bid,
        .ask = update.ask,
    });
};

void FullVisitor::push_orderbook_entry(const coinbase::Full& full, OrderBook::Entry&& entry) {
    switch (map_side(full.side)) {
    case Side::bid:
        push_orderbook_update(full, OrderBook::Update{.bid = entry});
        break;
    case Side::ask:
        push_orderbook_update(full, OrderBook::Update{.ask = entry});
        break;
    };
};

bool Source::find_product(const std::string& product_id) const {
    return std::find(_products.begin(), _products.end(), product_id) != _products.end();
}

CoinbaseSource::CoinbaseSource(boost::log::sources::logger_mt& logger, coinbase::Client& client, std::vector<std::string> products, std::size_t subscriber_buffer_size): Source{products}, _logger{logger}, _client{client}, _full_visitor{}, _orderbook_dispatcher{subscriber_buffer_size}, _trade_dispatcher{subscriber_buffer_size} {

};

bool CoinbaseSource::get_orderbook(const std::string& product_id, std::function<void (const OrderBook&)> callback) {
    std::unique_lock lock{_mtx};

    if (!_ready) {
        return false;
    };

    return _orderbooks.get(product_id, callback);
};

std::shared_ptr<Subscriber<OrderBook::Update>> CoinbaseSource::subscribe_orderbook(const std::string& product_id) {
    return _orderbook_dispatcher.subscribe([=](const auto& orderbook) { return orderbook.product_id == product_id; });
};

std::shared_ptr<Subscriber<Trade>> CoinbaseSource::subscribe_trade(const std::string& product_id) {
    return _trade_dispatcher.subscribe([=](const auto& trade) { return trade.product_id == product_id; });
};

bool CoinbaseSource::ready() {
    std::unique_lock lock{_mtx};

    return _ready;
};

void CoinbaseSource::run() {
    TaskGroup group;
    group.emplace(subscribe_full());

    fetch_orderbooks();

    group.launch([this] { dispatch_orderbook(); });
    group.launch([this] { dispatch_trade(); });

    group.get();
};

std::future<void> CoinbaseSource::subscribe_full() {
    auto&& res = _client.subscribe_full(products(), [this](const auto& full){ _full_visitor.apply(full); });
    BOOST_LOG(_logger) << "subscribed to full channel";

    return std::move(res);
};

void CoinbaseSource::fetch_orderbooks() {
    TaskGroup group;

    for (auto product: products()) {
        group.launch([this, product]() {
            auto orderbook = _client.get_orderbook(product);
            _orderbooks.emplace(std::string(product), map_orderbook(orderbook));

            BOOST_LOG(_logger) << "retrieved orderbook " << product;
        });
    };

    group.get();

    std::unique_lock lock{_mtx};
    _ready = true;
};

void CoinbaseSource::dispatch_orderbook() {
    try {
        while (true) {
            auto update = _full_visitor.pop_orderbook();

            auto update_res = _orderbooks.update(update);
            if (!update_res) {
                continue;
            };

            _orderbook_dispatcher.dispatch(update_res.value());
        };
    } catch (...) {
        std::throw_with_nested(std::runtime_error("dispatch_orderbook() failed"));
    };
};

void CoinbaseSource::dispatch_trade() {
    try {
        while (true) {
            auto trade = _full_visitor.pop_trade();
            _trade_dispatcher.dispatch(trade);
        };
    } catch (const std::exception& exc) {
        BOOST_LOG(_logger) << exc.what();
        std::throw_with_nested(std::runtime_error("dispatch_trade() failed"));
    };
}