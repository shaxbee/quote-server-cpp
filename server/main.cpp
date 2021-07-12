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

    std::string server_address = "0.0.0.0:8080";

    coinbase::ClientImpl client{ioc, "api-public.sandbox.pro.coinbase.com", "ws-feed-public.sandbox.pro.coinbase.com"};
    CoinbaseSource source{client};
    QuoteServiceImpl service(source);

    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    std::async(std::launch::async, [&] { source.run({"BTC-USD"}); }).get();

    return 0;
}
