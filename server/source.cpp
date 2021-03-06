#include "source.h"

#include <boost/log/common.hpp>
#include <boost/range/adaptors.hpp>

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

FullVisitor::FullVisitor(std::size_t buffer_size): _orderbook_buffer{buffer_size}, _trade_buffer{buffer_size} {

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

    _trade_buffer.push({
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

PopResult<OrderBook::Update> FullVisitor::pop_orderbook() {
    return _orderbook_buffer.pop();
};

PopResult<Trade> FullVisitor::pop_trade() {
    return _trade_buffer.pop();
}

void FullVisitor::push_orderbook_update(const coinbase::Full& full, OrderBook::Update&& update) {
    _orderbook_buffer.push({
        .product_id = full.product_id,
        .sequence = full.sequence,
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

CoinbaseSource::CoinbaseSource(boost::log::sources::logger_mt& logger, coinbase::Client& client, std::vector<std::string> products, std::size_t subscriber_buffer_size, std::size_t channel_buffer_size): Source{products}, _logger{logger}, _client{client}, _full_visitor{channel_buffer_size}, _orderbook_dispatcher{subscriber_buffer_size}, _trade_dispatcher{subscriber_buffer_size} {

};

bool CoinbaseSource::get_orderbook(const std::string& product_id, std::function<void (const OrderBook&)> callback) {
    std::unique_lock lock{_mtx};

    if (!_ready) {
        return false;
    };

    return _orderbooks->get(product_id, callback);
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
    std::vector<std::future<void>> tasks;

    tasks.emplace_back(subscribe_full());

    fetch_orderbooks();

    tasks.emplace_back(std::async(std::launch::async, [this] { dispatch_orderbook(); }));
    tasks.emplace_back(std::async(std::launch::async, [this] { dispatch_trade(); }));

    while (tasks.size()) {
        for (auto it = tasks.begin(); it != tasks.end(); it++) {
            auto& task = *it;

            auto status = task.wait_for(std::chrono::milliseconds(100));
            if (status != std::future_status::ready) {
                continue;
            };

            task.get();

            tasks.erase(it);
        };
    };
};

std::future<void> CoinbaseSource::subscribe_full() {
    try {
    auto&& res = _client.subscribe_full(products(), [this](const auto& full){ _full_visitor.apply(full); });
    BOOST_LOG(_logger) << "subscribed to full channel";

    return std::move(res);
    } catch (...) {
        std::throw_with_nested(std::runtime_error("subscribe_full() failed"));
    }
};

void CoinbaseSource::fetch_orderbooks() {
    std::unordered_map<std::string, OrderBook> orderbooks;

    for (auto product: products()) {
        auto orderbook = _client.get_orderbook(product);
        orderbooks.emplace(product, map_orderbook(orderbook));

        BOOST_LOG(_logger) << "retrieved orderbook " << product;
    };

    std::unique_lock lock{_mtx};
    _orderbooks = std::make_unique<OrderBooks>(std::move(orderbooks));
    _ready = true;
};

void CoinbaseSource::dispatch_orderbook() {
    try {
        while (true) {
            auto [res, state] = _full_visitor.pop_orderbook();
            if (state == PopState::overflow) {
                throw std::invalid_argument("orderbook buffer overflow");
            };

            auto update = _orderbooks->update(*res);
            if (!update) {
                continue;
            };

            _orderbook_dispatcher.dispatch(*update);
        };
    } catch (...) {
        std::throw_with_nested(std::runtime_error("dispatch_orderbook() failed"));
    };
};

void CoinbaseSource::dispatch_trade() {
    try {
        while (true) {
            auto [res, state] = _full_visitor.pop_trade();
            if (state == PopState::overflow) {
                throw std::invalid_argument("trade buffer overflow");
            };

            _trade_dispatcher.dispatch(*res);
        };
    } catch (...) {
        std::throw_with_nested(std::runtime_error("dispatch_trade() failed"));
    };
}