#ifndef COINBASE_CLIENT_H
#define COINBASE_CLIENT_H 1

#include <cstdint>
#include <functional>
#include <future>
#include <string>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>

#include "orderbook.h"
#include "full.h"

namespace coinbase {

struct Client {
   virtual OrderBook get_orderbook(std::string product) = 0;
   virtual std::future<void> subscribe_full(std::vector<std::string> products, std::function<void(const Full&)> callback) = 0;
};

class ClientImpl: public Client {
public:
   ClientImpl(boost::asio::io_context& ioc, std::string rest_host, std::string websocket_host);

   virtual OrderBook get_orderbook(std::string product);
   virtual std::future<void> subscribe_full(std::vector<std::string> products, std::function<void(const Full&)> callback);
private:
   const std::string rest_host;
   const std::string websocket_host;
   boost::asio::io_context& ioc;
   boost::asio::ssl::context sslc;
   boost::asio::ip::tcp::resolver resolver;
};

} // namespace coinbase

#endif