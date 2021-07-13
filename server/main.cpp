#include <csignal>
#include <cstdlib>
#include <future>
#include <memory>

#include <boost/algorithm/string.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/log/common.hpp>
#include <boost/log/sources/logger.hpp>

#include <grpcpp/grpcpp.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

#include "quote_service.h"
#include "coinbase/client.h"

struct Config {
    std::string addr;
    std::string rest_endpoint;
    std::string websocket_endpoint;
    std::vector<std::string> products;

    static Config from_env();
};

int main() {
    std::signal(SIGINT, [](int sig) { std::exit(1); });

    boost::log::sources::logger_mt logger;

    auto config = Config::from_env();

    boost::asio::io_context ioc;
    coinbase::ClientImpl client{ioc, config.rest_endpoint, config.websocket_endpoint};
    CoinbaseSource source{logger, client, config.products};
    QuoteServiceImpl service(source);

    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    grpc::ServerBuilder builder;
    builder.SetMaxSendMessageSize(10 * 1024 * 1024);
    builder.AddListeningPort(config.addr, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    BOOST_LOG(logger) << "Server listening on " << config.addr;

    std::async(std::launch::async, [&] { source.run(); }).get();

    return 0;
};

Config Config::from_env() {
    auto addr = std::getenv("QS_ADDR");
    auto rest_endpoint = std::getenv("QS_COINBASE_REST_ENDPOINT");
    auto websocket_endpoint = std::getenv("QS_COINBASE_WEBSOCKET_ENDPOINT");
    auto raw_products = std::getenv("QS_PRODUCTS");

    std::vector<std::string> products;
    if (raw_products != nullptr) {
        boost::algorithm::split(products, std::string(raw_products), boost::algorithm::is_any_of(","));
    } else {
        products = {"BTC-USD"};
    };

    return Config{
        .addr = (addr != nullptr ? addr : "0.0.0.0:8080"),
        .rest_endpoint = (rest_endpoint != nullptr ? rest_endpoint : "api-public.sandbox.pro.coinbase.com"),
        .websocket_endpoint = (websocket_endpoint != nullptr ? websocket_endpoint : "ws-feed-public.sandbox.pro.coinbase.com"),
        .products = products,
    };
}
