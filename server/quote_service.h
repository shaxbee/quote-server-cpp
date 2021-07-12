#ifndef QUOTE_SERVICE_H
#define QUOTE_SERVICE_H 1

#include <memory>
#include <mutex>
#include <unordered_map>

#include <grpcpp/grpcpp.h>

#include "quote.grpc.pb.h"

#include "source.h"

using OrderBookDispatcher = Dispatcher<OrderBook::Update>;

class QuoteServiceImpl final : public quote::Quote::Service {
public:
    QuoteServiceImpl(Source& source);

    grpc::Status SubscribeOrderBook(grpc::ServerContext* context, const quote::SubscribeOrderBookRequest* request, grpc::ServerWriter<quote::OrderBook>* writer);
    grpc::Status SubscribeTrade(grpc::ServerContext* context, const quote::SubscribeTradeRequest* request, grpc::ServerWriter<quote::Trade>* writer);

private:
    Source& _source;
};

#endif