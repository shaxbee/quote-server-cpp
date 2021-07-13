#ifndef SERVER_SOURCE_H
#define SERVER_SOURCE_H 1

#include <future>
#include <mutex>

#include <boost/log/sources/logger.hpp>

#include "dispatcher.h"
#include "orderbook.h"
#include "ring_buffer.h"
#include "trade.h"

#include "coinbase/client.h"
#include "coinbase/full.h"

class Source {
public:
    explicit Source(std::vector<std::string> products): _products{std::move(products)} { }

    inline const std::vector<std::string>& products() const { return _products; };
    bool find_product(const std::string& product) const;

    virtual bool get_orderbook(const std::string& product_id, std::function<void (const OrderBook&)> callback) = 0;
    virtual std::shared_ptr<Subscriber<OrderBook::Update>> subscribe_orderbook(const std::string& product_id) = 0;
    virtual std::shared_ptr<Subscriber<Trade>> subscribe_trade(const std::string& product_id) = 0;

    virtual void run() = 0;
    virtual bool ready() = 0;

private:
    const std::vector<std::string> _products;
};

class FullVisitor: public coinbase::FullVisitor {
public:
    FullVisitor(std::size_t buffer_size);

    PopResult<OrderBook::Update> pop_orderbook();
    PopResult<Trade> pop_trade();

    void visit(const coinbase::Full& full, const coinbase::Received& received) override;
    void visit(const coinbase::Full& full, const coinbase::Open& open) override;
    void visit(const coinbase::Full& full, const coinbase::Done& done) override;
    void visit(const coinbase::Full& full, const coinbase::Match& match) override;
    void visit(const coinbase::Full& full, const coinbase::Change& change) override;

private:
    RingBuffer<OrderBook::Update> _orderbook_buffer;
    RingBuffer<Trade> _trade_buffer;

    void push_orderbook_update(const coinbase::Full& full, OrderBook::Update&& update);
    void push_orderbook_entry(const coinbase::Full& full, OrderBook::Entry&& entry);
};

class CoinbaseSource: public Source {
public:
    CoinbaseSource(boost::log::sources::logger_mt& logger, coinbase::Client& client, std::vector<std::string> products, std::size_t subscriber_buffer_size = 1024, std::size_t channel_buffer_size = 65536);

    bool get_orderbook(const std::string& product_id, std::function<void (const OrderBook&)> callback) override;
    std::shared_ptr<Subscriber<OrderBook::Update>> subscribe_orderbook(const std::string& product_id) override;
    std::shared_ptr<Subscriber<Trade>> subscribe_trade(const std::string& product_id) override;

    void run() override;
    bool ready() override;

private:
    std::mutex _mtx;

    coinbase::Client& _client;
    boost::log::sources::logger_mt& _logger;
    FullVisitor _full_visitor;
    Dispatcher<OrderBook::Update> _orderbook_dispatcher;
    Dispatcher<Trade> _trade_dispatcher;

    std::unique_ptr<OrderBooks> _orderbooks;
    bool _ready;

    std::future<void> subscribe_full();
    void fetch_orderbooks();
    void dispatch_orderbook();
    void dispatch_trade();
};

#endif