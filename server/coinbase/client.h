#ifndef COINBASE_CLIENT_H
#define COINBASE_CLIENT_H 1

#include <cstdint>
#include <string>

#include "orderbook.h"

namespace coinbase {

class BaseClient {
public:
   virtual OrderBook GetOrderbook(std::string product) = 0;
   virtual void SubscribeFull(std::vector<std::string> products) = 0;
};

} // namespace coinbase

#endif