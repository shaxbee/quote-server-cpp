#include <future>
#include <iostream>
#include <memory>

#include <boost/asio/io_context.hpp>

#include <grpcpp/grpcpp.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

#include "quote_service.h"
#include "coinbase/client.h"

int main() {
    boost::asio::io_context ioc;
    auto client = coinbase::Client(ioc, "api-public.sandbox.pro.coinbase.com", "ws-feed-public.sandbox.pro.coinbase.com");
    client.subscribe_full({"BTC-USD"}, [](auto full) {
        if (full.type == coinbase::Full::Type::Open) {
            std::cout << full.as_open() << std::endl;
        };
    });
    // std::cout << orderbook << std::endl;
    // std::string server_address = "0.0.0.0:8080";

    // QuoteServiceImpl service;

    // grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    // grpc::ServerBuilder builder;
    // builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // builder.RegisterService(&service);

    // std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    // std::cout << "Server listening on " << server_address << std::endl;

    // service.Start();

    // server->Wait();

    return 0;
}
