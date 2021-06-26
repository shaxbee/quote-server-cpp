#ifndef QUOTE_SERVICE_H
#define QUOTE_SERVICE_H 1

#include <atomic>
#include <future>

#include <grpcpp/grpcpp.h>

#include "quote.grpc.pb.h"
#include "dispatcher.h"

class QuoteServiceImpl final : public quote::Quote::Service {
public:
    QuoteServiceImpl(): orderBook(16), trades(16) {};

    void Start();
    void Wait();

    grpc::Status SubscribeOrderBook(grpc::ServerContext* context, const quote::SubscribeOrderBookRequest* request, grpc::ServerWriter<quote::OrderBook>* writer);
    grpc::Status SubscribeTrade(grpc::ServerContext* context, const quote::SubscribeTradeRequest* request, grpc::ServerWriter<quote::Trade>* writer);

private:
    std::future<void> worker;

    Dispatcher<quote::OrderBook> orderBook;
    Dispatcher<quote::Trade> trades;
};

#endif