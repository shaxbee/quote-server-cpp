#include "quote_service.h"

#include <future>
#include <variant>

#include <boost/range/adaptors.hpp>

namespace {

quote::OrderBookEntry map_orderbook_entry(const OrderBook::Entry& src) {
    quote::OrderBookEntry dst;

    dst.set_order_id(src.order_id);
    dst.set_price(src.price.str());
    dst.set_quantity(src.size.str());

    return dst;
};

quote::OrderBook map_orderbook(const std::string& product_id, const OrderBook& src) {
    quote::OrderBook dst;

    dst.set_product_id(product_id);
    dst.set_sequence(src.sequence());

    auto asks = dst.mutable_asks();
    for (const auto& entry: src.bids() | boost::adaptors::map_values) {
        asks->Add(map_orderbook_entry(entry));
    };

    auto bids = dst.mutable_bids();
    for (const auto& entry: src.asks() | boost::adaptors::map_values) {
        bids->Add(map_orderbook_entry(entry));
    };

    return dst;
};

quote::OrderBook map_orderbook_update(const OrderBook::Update& src) {
    quote::OrderBook dst;

    dst.set_product_id(src.product_id);
    dst.set_sequence(src.sequence);
    dst.set_time(src.time);

    quote::OrderBookEntry* entry;

    if (src.bid) {
        dst.mutable_bids()->Add(map_orderbook_entry(*src.bid));
    };

    if (src.ask) {
        dst.mutable_asks()->Add(map_orderbook_entry(*src.ask));
    };

    return dst;
};

quote::Trade map_trade(const Trade& src) {
    quote::Trade dst;

    dst.set_product_id(src.product_id);
    dst.set_time(src.time);

    switch (src.side) {
    case Side::bid:
        dst.set_side(quote::Side::BID); break;
    case Side::ask:
        dst.set_side(quote::Side::ASK); break;
    };

    dst.set_maker_order_id(src.maker_order_id);
    dst.set_taker_order_id(src.taker_order_id);
    dst.set_price(src.price.str());
    dst.set_size(src.size.str());

    return dst;
};

} // anonymous namespace

QuoteServiceImpl::QuoteServiceImpl(Source& source): _source(source) {

};

grpc::Status QuoteServiceImpl::SubscribeOrderBook(grpc::ServerContext* context, const quote::SubscribeOrderBookRequest* request, grpc::ServerWriter<quote::OrderBook>* writer) {
    if (!_source.ready()) {
        return grpc::Status(grpc::StatusCode::UNAVAILABLE, "Unavailable");
    };

    auto product_id = request->product_id();
    auto subscriber = _source.subscribe_orderbook(product_id);


    quote::OrderBook orderbook;
    auto found = _source.get_orderbook(product_id, [&](auto src) {
        orderbook = std::move(map_orderbook(product_id, src));
    });

    if (!found) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "OrderBook not found");
    };

    // send the snapshot
    if (!writer->Write(orderbook)) {
        return grpc::Status::CANCELLED;
    }

    auto sequence = orderbook.sequence();

    while (!context->IsCancelled()) {
        auto [res, state] = subscriber->pop_front_wait(std::chrono::seconds(1));

        // slow consumer
        if (state == PopState::overflow) {
            return grpc::Status(grpc::StatusCode::DEADLINE_EXCEEDED, "slow consumer");
        };

        // no update received
        if (state == PopState::timeout) {
            continue;
        };
        
        auto update = *res;

        // ignore updates that are already in orderbook
        if (update.sequence <= sequence) {
            continue;
        }

        if (!writer->Write(map_orderbook_update(update))) {
            break;   
        };

        sequence = update.sequence;
    }
    
    return grpc::Status::CANCELLED;
};

grpc::Status QuoteServiceImpl::SubscribeTrade(grpc::ServerContext* context, const quote::SubscribeTradeRequest* request, grpc::ServerWriter<quote::Trade>* writer) {
    if (!_source.ready()) {
        return grpc::Status(grpc::StatusCode::UNAVAILABLE, "Unavailable");
    };


    auto product_id = request->product_id();
    if (!_source.find_product(product_id)) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "Product not found");
    };

    auto subscriber = _source.subscribe_trade(product_id);

    while (!context->IsCancelled()) {
        auto [res, state] = subscriber->pop_front_wait(std::chrono::seconds(1));

        // slow consumer
        if (state == PopState::overflow) {
            return grpc::Status(grpc::StatusCode::DEADLINE_EXCEEDED, "slow consumer");
        };

        // no update received
        if (state == PopState::timeout) {
            continue;
        };

        if (!writer->Write(map_trade(*res))) {
            break;   
        };
    };
    
    return grpc::Status::CANCELLED;
};