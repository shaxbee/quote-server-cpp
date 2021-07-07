#ifndef COINBASE_FULL_H
#define COINBASE_FULL_H 1

#include <any>
#include <cstdint>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <boost/json.hpp>

#include "../decimal.h"
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
    std::any value;

    struct Received {
        std::string time;
        std::string product_id;
        std::int64_t sequence;
        std::string order_id;
        std::string order_type;
        std::string side;
        Decimal size;
        Decimal price;
        Decimal funds;

        bool operator==(const Received&) const = default;
    };

    struct Open {
        std::string time;
        std::string product_id;
        std::int64_t sequence;
        std::string order_id;
        std::string side;
        Decimal price;
        Decimal remaining_size;

        bool operator==(const Open&) const = default;
    };

    struct Done {
        std::string time;
        std::string product_id;
        std::int64_t sequence;
        std::string order_id;
        std::string side;
        Decimal price;
        Decimal remaining_size;
        std::string reason;

        bool operator==(const Done&) const = default;
    };

    struct Match {
        std::string time;
        std::string product_id;
        std::int64_t sequence;
        std::string maker_order_id;
        std::string taker_order_id;
        std::string side;
        Decimal price;
        Decimal size;

        bool operator==(const Match&) const = default;
    };

    struct Change {
        std::string time;
        std::string product_id;
        std::int64_t sequence;
        std::string order_id;
        std::string side;
        Decimal price;
        Decimal old_size;
        Decimal new_size;
        Decimal old_funds;
        Decimal new_funds;

        bool operator==(const Change&) const = default;
    };

    struct Activate {};

    Received as_received() const {
        return std::any_cast<Received>(value);
    }

    Open as_open() const {
        return std::any_cast<Open>(value);
    }

    Done as_done() const {
        return std::any_cast<Done>(value);
    }

    Match as_match() const {
        return std::any_cast<Match>(value);
    }

    Change as_change() const {
        return std::any_cast<Change>(value);
    }
};

Full parse_full(std::string data);

std::ostream& operator<<(std::ostream& os, const Full::Received& v);
std::ostream& operator<<(std::ostream& os, const Full::Open& v);
std::ostream& operator<<(std::ostream& os, const Full::Done& v);
std::ostream& operator<<(std::ostream& os, const Full::Match& v);
std::ostream& operator<<(std::ostream& os, const Full::Change& v);

} // namespace coinbase

#endif