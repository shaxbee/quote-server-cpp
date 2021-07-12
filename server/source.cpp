#include "source.h"

#include <iostream>
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

FullVisitor::FullVisitor(std::size_t buffer_size): _orderbook_buffer(buffer_size) {

};

void FullVisitor::visit(const coinbase::Full& full, const coinbase::Open& open) {
    push_orderbook(full, OrderBook::Entry{
        .order_id = open.order_id,
        .price = open.price,
        .size = open.remaining_size,
    });
};

void FullVisitor::visit(const coinbase::Full& full, const coinbase::Done& done) {
    // market orders are never on orderbook and have price 0
    if (!done.price) {
        return;
    };

    push_orderbook(full, OrderBook::Entry{
        .order_id = done.order_id,
        .price = done.price,
    });
};

void FullVisitor::visit(const coinbase::Full& full, const coinbase::Match& match) {
    push_orderbook(full, OrderBook::Entry{
        .order_id = match.maker_order_id,
        .size = -match.size,
    });
};

PopResult<OrderBook::Update> FullVisitor::pop_orderbook() {
    return _orderbook_buffer.pop();
};

void FullVisitor::push_orderbook(const coinbase::Full& full, OrderBook::Update&& update) {
    _orderbook_buffer.push({
        .product_id = full.product_id,
        .sequence = full.sequence,
        .bid = update.bid,
        .ask = update.ask,
    });
};

void FullVisitor::push_orderbook(const coinbase::Full& full, OrderBook::Entry&& entry) {
    switch (map_side(full.side)) {
    case Side::bid:
        push_orderbook(full, OrderBook::Update{.bid = entry});
        break;
    case Side::ask:
        push_orderbook(full, OrderBook::Update{.ask = entry});
        break;
    };
};

CoinbaseSource::CoinbaseSource(coinbase::Client& client, std::size_t subscriber_buffer_size, std::size_t channel_buffer_size): _client(client), _full_visitor(channel_buffer_size), _orderbook_dispatcher(subscriber_buffer_size) {

};

bool CoinbaseSource::get_orderbook(const std::string& product_id, std::function<void (const OrderBook&)> callback) {
    std::unique_lock lock{_mtx};

    if (!_ready) {
        return false;
    };

    return _orderbooks->get(product_id, callback);
};

std::shared_ptr<Subscriber<OrderBook::Update>> CoinbaseSource::subscribe_orderbook() {
    return _orderbook_dispatcher.subscribe();
};

bool CoinbaseSource::ready() {
    std::unique_lock lock{_mtx};

    return _ready;
}

void CoinbaseSource::run(const std::vector<std::string>& products) {
    std::vector<std::future<void>> tasks;

    tasks.emplace_back(subscribe_full(products));

    fetch_orderbooks(products);

    tasks.emplace_back(std::async(std::launch::async, [this]() {
        try {
            dispatch_orderbook_updates();
        } catch (const std::exception& exc) {
            std::cout << "dispatch_orderbook_updates: " << exc.what() << std::endl;
        };
    }));

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

std::future<void> CoinbaseSource::subscribe_full(const std::vector<std::string>& products) {
    return _client.subscribe_full(products, [this](const auto& full){ _full_visitor.apply(full); });
};

void CoinbaseSource::fetch_orderbooks(const std::vector<std::string>& products) {
    std::unordered_map<std::string, OrderBook> orderbooks;

    for (auto product: products) {
        auto orderbook = _client.get_orderbook(product);
        orderbooks.emplace(product, map_orderbook(orderbook));
    };

    std::unique_lock lock{_mtx};
    _orderbooks = std::make_unique<OrderBooks>(std::move(orderbooks));
    _ready = true;
};

void CoinbaseSource::dispatch_orderbook_updates() {
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
};