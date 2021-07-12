#ifndef SERVER_SOURCE_H
#define SERVER_SOURCE_H 1

#include <future>
#include <mutex>

#include "dispatcher.h"
#include "orderbook.h"
#include "ring_buffer.h"

#include "coinbase/client.h"
#include "coinbase/full.h"

struct Source {
    virtual bool get_orderbook(const std::string& product_id, std::function<void (const OrderBook&)> callback) = 0;
    virtual std::shared_ptr<Subscriber<OrderBook::Update>> subscribe_orderbook() = 0;

    virtual void run(const std::vector<std::string>& products) = 0;
    virtual bool ready() = 0;
};

class FullVisitor: public coinbase::FullVisitor {
public:
    FullVisitor(std::size_t buffer_size);

    PopResult<OrderBook::Update> pop_orderbook();

    void visit(const coinbase::Full& full, const coinbase::Open& open);
    void visit(const coinbase::Full& full, const coinbase::Done& done);
    void visit(const coinbase::Full& full, const coinbase::Match& match);

private:
    RingBuffer<OrderBook::Update> _orderbook_buffer;

    void push_orderbook(const coinbase::Full& full, OrderBook::Update&& update);
    void push_orderbook(const coinbase::Full& full, OrderBook::Entry&& entry);
};

class CoinbaseSource: public Source {
public:
    CoinbaseSource(coinbase::Client& client, std::size_t subscriber_buffer_size = 32, std::size_t channel_buffer_size = 1024);

    bool get_orderbook(const std::string& product_id, std::function<void (const OrderBook&)> callback) override;
    std::shared_ptr<Subscriber<OrderBook::Update>> subscribe_orderbook() override;

    void run(const std::vector<std::string>& products) override;
    bool ready() override;

private:
    std::mutex _mtx;
    coinbase::Client& _client;
    FullVisitor _full_visitor;
    Dispatcher<OrderBook::Update> _orderbook_dispatcher;

    std::unique_ptr<OrderBooks> _orderbooks;
    bool _ready;

    std::future<void> subscribe_full(const std::vector<std::string>& products);
    void fetch_orderbooks(const std::vector<std::string>& products);
    void dispatch_orderbook_updates();
};

#endif