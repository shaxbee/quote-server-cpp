#ifndef COINBASE_FULL_H
#define COINBASE_FULL_H 1

#include <any>
#include <cstdint>
#include <string>

#include <boost/json.hpp>

#include "decimal.h"
#include "time.h"

namespace coinbase {

struct Full {
    enum class Type: int {
        Received,
        Open,
        Done,
        Match,
        Change,
        Activate
    };

    Type type;
    std::string time;
    std::string product_id;
    std::int64_t sequence;
    std::string side;
    std::any value;

    struct Received {};

    struct Open {
        std::string order_id;
        Decimal price;
        Decimal remaining_size;
    };

    struct Done {
        std::string order_id;
        Decimal price;
        Decimal remaining_size;
        std::string reason;
    };

    struct Match {
        std::string maker_order_id;
        std::string taker_order_id;
        Decimal price;
        Decimal size;
    };

    struct Change {
        std::string order_id;
        Decimal price;
        Decimal old_size;
        Decimal new_size;
    };

    struct Activate {};
};

Full tag_invoke(boost::json::value_to_tag<Full>, boost::json::value const& src);
Full::Type tag_invoke(boost::json::value_to_tag<Full::Type>, boost::json::value const& src);
Full::Received tag_invoke(boost::json::value_to_tag<Full::Received>, boost::json::value const& src);
Full::Open tag_invoke(boost::json::value_to_tag<Full::Open>, boost::json::value const& src);
Full::Done tag_invoke(boost::json::value_to_tag<Full::Done>, boost::json::value const& src);
Full::Match tag_invoke(boost::json::value_to_tag<Full::Match>, boost::json::value const& src);
Full::Change tag_invoke(boost::json::value_to_tag<Full::Change>, boost::json::value const& src);
Full::Activate tag_invoke(boost::json::value_to_tag<Full::Activate>, boost::json::value const& src);

} // namespace coinbase

#endif