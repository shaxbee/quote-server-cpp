#include "subscriptions.h"

#include <stdexcept>

#include <boost/json.hpp>

namespace coinbase {

Subscriptions parse_subscriptions(std::string data) {
    boost::json::parser p;
    p.write(data);

    return value_to<Subscriptions>(p.release());  
};

Subscriptions tag_invoke(boost::json::value_to_tag<Subscriptions>, boost::json::value const& src) {
    auto type = boost::json::value_to<std::string>(src.at("type"));
    if (type != "subscriptions") {
        throw std::invalid_argument("expected subscriptions message");
    };

    return Subscriptions{
        .channels{boost::json::value_to<std::vector<Channel>>(src.at("channels"))}
    };
};

Channel tag_invoke(boost::json::value_to_tag<Channel>, boost::json::value const& src) {
    return Channel{
        .name{boost::json::value_to<std::string>(src.at("name"))},
        .product_ids{boost::json::value_to<std::vector<std::string>>(src.at("product_ids"))}
    };
}

} // namespace coinbase