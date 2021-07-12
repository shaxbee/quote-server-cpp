#include "client.h"

#include <iostream>
#include <future>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include "subscriptions.h"

namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;

using tcp = boost::asio::ip::tcp;

namespace coinbase {

ClientImpl::ClientImpl(boost::asio::io_context& ioc, std::string rest_host, std::string websocket_host): rest_host(rest_host), websocket_host(websocket_host), ioc(ioc), sslc(ssl::context::sslv23), resolver(ioc) {
    sslc.set_default_verify_paths();
};

OrderBook ClientImpl::get_orderbook(std::string product) {
    beast::ssl_stream<beast::tcp_stream> stream(ioc, sslc);

    // setup SNI
    if (!SSL_set_tlsext_host_name(stream.native_handle(), rest_host.c_str())) {
        beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
        throw beast::system_error{ec};
    }

    // resolve https address
    auto const results = resolver.resolve(rest_host, "https");

    // connect and perform ssl handshake
    beast::get_lowest_layer(stream).connect(results);
    stream.handshake(ssl::stream_base::client);

    // write request
    http::request<http::string_body> req{http::verb::get, "/products/" + product + "/book?level=3", 11};
    req.set(http::field::host, rest_host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    http::write(stream, req);

    // read response
    beast::flat_buffer buffer;
    http::response<http::string_body> res;

    http::read(stream, buffer, res);

    beast::error_code ec;
    stream.shutdown(ec);
    if (ec && ec != net::error::eof && ec != ec == ssl::error::stream_truncated) {
        throw boost::beast::system_error{ec};
    }

    return parse_orderbook(res.body());
};

std::future<void> ClientImpl::subscribe_full(std::vector<std::string> products, std::function<void(const Full&)> callback) {
    websocket::stream<beast::ssl_stream<beast::tcp_stream>> stream{ioc, sslc};

    // setup SNI
    if (!SSL_set_tlsext_host_name(stream.next_layer().native_handle(), websocket_host.c_str())) {
        beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
        throw beast::system_error{ec};
    }

    // resolve https address
    auto const results = resolver.resolve(websocket_host, "https");

    // connect and perform ssl handshake
    auto ep = beast::get_lowest_layer(stream).connect(results);
    stream.next_layer().handshake(ssl::stream_base::client);

    auto host = websocket_host + ':' + std::to_string(ep.port());

    stream.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req)
            {
                req.set(http::field::user_agent,BOOST_BEAST_VERSION_STRING);
            }));

    stream.handshake(host, "/");

    auto subscribe = serialize_subscribe({
        .channels = {
            {.name = "full", .product_ids = products}
        },
    });
    stream.write(net::buffer(std::string(subscribe)));

    beast::flat_buffer buffer;

    stream.read(buffer);
    auto data = beast::buffers_to_string(buffer.cdata());
    parse_subscriptions(data);

    // this fails due to stream going out of scope
    return std::async(std::launch::async, [callback, stream = std::move(stream)]() mutable {
        while (true) {
            beast::flat_buffer buffer;

            buffer.clear();
            stream.read(buffer);

            auto data = beast::buffers_to_string(buffer.cdata());
            auto full = parse_full(data);

            callback(full);
        };

        try {
            stream.close(websocket::close_code::normal);
        } catch (boost::beast::system_error exc) {
            if (exc.code() != ssl::error::stream_truncated) {
                throw exc;
            };
        };
    });
};

} // namespace coinbase