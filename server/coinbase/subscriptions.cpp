#include "subscriptions.h"

#include <stdexcept>

#include <boost/json.hpp>

namespace coinbase {

Subscriptions parse_subscriptions(std::string data) {
    return value_to<Subscriptions>(boost::json::parse(data));  
};

std::string serialize_subscribe(Subscribe subscribe) {
    return boost::json::serialize(boost::json::value_from(subscribe));
}

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
};

void tag_invoke(boost::json::value_from_tag, boost::json::value& dst, const Channel& src) {
    dst = {
        {"name", src.name},
        {"product_ids", src.product_ids},
    };
};

void tag_invoke(boost::json::value_from_tag, boost::json::value& dst, const Subscribe& src) {
    dst = {
        {"type", "subscribe"},
        {"channels", src.channels},
    };
};

} // namespace coinbase