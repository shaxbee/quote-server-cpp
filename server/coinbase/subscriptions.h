#ifndef COINBASE_SUBSCRIPTIONS_H
#define COINBASE_SUBSCRIPTIONS_H 1

#include <string>
#include <vector>

namespace coinbase {

struct Channel {
    std::string name;
    std::vector<std::string> product_ids;
};

struct Subscriptions {
    std::vector<Channel> channels;
};

Subscriptions parse_subscriptions(std::string data);

} // namespace coinbase

#endif