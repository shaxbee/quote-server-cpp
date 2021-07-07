#include "quote_service.h"

#include <iostream>
#include <thread>

void QuoteServiceImpl::Start() {
    worker = std::async(std::launch::async, [this] {
        int sequence = 0;
        while(true) {
            quote::OrderBook v;
            v.set_sequence(sequence);

            orderBook.dispatch(v);

            std::this_thread::sleep_for(std::chrono::seconds(1));
            sequence++;
        }
    });
}

void QuoteServiceImpl::Wait() {
    worker.wait();
}

grpc::Status QuoteServiceImpl::SubscribeOrderBook(grpc::ServerContext* context, const quote::SubscribeOrderBookRequest* request, grpc::ServerWriter<quote::OrderBook>* writer) {
    auto subscriber = orderBook.subscribe();

    while (!context->IsCancelled()) {
        auto [res, state] = subscriber->pop(std::chrono::seconds(1));
        switch (state) {
        case PopState::overflow:
            return grpc::Status(grpc::StatusCode::DEADLINE_EXCEEDED, "slow consumer");
        case PopState::timeout:
            continue;
        case PopState::valid:;
            writer->Write(res.value());
        }
    }
    
    return grpc::Status::CANCELLED;
}

grpc::Status QuoteServiceImpl::SubscribeTrade(grpc::ServerContext* context, const quote::SubscribeTradeRequest* request, grpc::ServerWriter<quote::Trade>* writer) {
    return grpc::Status::OK;
}