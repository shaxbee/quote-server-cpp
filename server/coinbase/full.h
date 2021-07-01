#ifndef COINBASE_FULL_H
#define COINBASE_FULL_H 1

#include <any>
#include <cstdint>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unordered_map>

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

Decimal decimal_from_key(const boost::json::object& obj, const char* key) {
    if (obj.contains(key)) {
        return Decimal{boost::json::value_to<std::string>(obj.at(key))};
    }

    return Decimal{};
}

static std::unordered_map<std::string, Full::Type> full_type_names = {
    {"received", Full::Type::Received},
    {"open", Full::Type::Open},
    {"done", Full::Type::Done},
    {"match", Full::Type::Match},
    {"change", Full::Type::Change},
    {"activate", Full::Type::Activate},
};

Full tag_invoke(boost::json::value_to_tag<Full>, boost::json::value const& src) {
    auto type = boost::json::value_to<Full::Type>(src.at("type"));
    std::any value;

    switch (type) {
    case Full::Type::Received:
        value = boost::json::value_to<Full::Received>(src);
        break;
    case Full::Type::Open:
        value = boost::json::value_to<Full::Open>(src);
        break;
    case Full::Type::Done:
        value = boost::json::value_to<Full::Done>(src);
        break;
    case Full::Type::Match:
        value = boost::json::value_to<Full::Match>(src);
        break;
    case Full::Type::Change:
        value = boost::json::value_to<Full::Change>(src);
        break;
    case Full::Type::Activate:
        value = boost::json::value_to<Full::Activate>(src);
        break;
    }

    return Full{
        .type = type,
        .value = value,
    };
}

Full::Type tag_invoke(boost::json::value_to_tag<Full::Type>, boost::json::value const& src) {
    auto it = full_type_names.find(boost::json::value_to<std::string>(src));
    if (it == full_type_names.end()) {
        throw std::invalid_argument("unsupported Full::Type value");
    };

    return it->second;
}


std::ostream& operator<<(std::ostream& os, const Full::Received& v) {
    return os << std::setprecision(std::numeric_limits<Decimal>::max_digits10) << "{" <<
        ".time=" << v.time << ", " <<
        ".product_id=" << v.product_id << ", " <<
        ".sequence=" << v.sequence << ", " <<
        ".order_id=" << v.order_id << ", " <<
        ".order_type=" << v.order_type << ", " <<
        ".side=" << v.side << ", " <<
        ".size=" << v.size << ", " <<
        ".price=" << v.price << ", " <<
        ".funds=" << v.funds <<
        "}";
}

Full::Received tag_invoke(boost::json::value_to_tag<Full::Received>, boost::json::value const& src) {
    auto obj = src.as_object();
    
    return Full::Received{
        .time = boost::json::value_to<std::string>(obj.at("time")),
        .product_id = boost::json::value_to<std::string>(obj.at("product_id")),
        .sequence = obj.at("sequence").as_int64(),
        .order_id = boost::json::value_to<std::string>(obj.at("order_id")),
        .side = boost::json::value_to<std::string>(obj.at("side")),
        .price = decimal_from_key(obj, "price"),
        .size = decimal_from_key(obj, "size"),
        .funds = decimal_from_key(obj, "funds"),
        .order_type = boost::json::value_to<std::string>(obj.at("order_type"))
    };
}

std::ostream& operator<<(std::ostream& os, const Full::Open& v) {
    return os << std::setprecision(std::numeric_limits<Decimal>::max_digits10) << "{" <<
        ".time=" << v.time << ", " <<
        ".product_id=" << v.product_id << ", " <<
        ".sequence=" << v.sequence << ", " <<
        ".order_id=" << v.order_id << ", " <<
        ".side=" << v.side << ", " <<
        ".price=" << v.price << ", " <<
        ".remaining_size=" << v.remaining_size <<
        "}";
}

Full::Open tag_invoke(boost::json::value_to_tag<Full::Open>, boost::json::value const& src) {
    return Full::Open{
        .time = boost::json::value_to<std::string>(src.at("time")),
        .product_id = boost::json::value_to<std::string>(src.at("product_id")),
        .sequence = src.at("sequence").as_int64(),
        .order_id = boost::json::value_to<std::string>(src.at("order_id")),
        .side = boost::json::value_to<std::string>(src.at("side")),
        .price = Decimal{boost::json::value_to<std::string>(src.at("price"))},
        .remaining_size = Decimal{boost::json::value_to<std::string>(src.at("remaining_size"))}
    };
}

std::ostream& operator<<(std::ostream& os, const Full::Done& v) {
    return os << std::setprecision(std::numeric_limits<Decimal>::max_digits10) << "{" <<
        ".time=" << v.time << ", " <<
        ".product_id=" << v.product_id << ", " <<
        ".sequence=" << v.sequence << ", " <<
        ".order_id=" << v.order_id << ", " <<
        ".side=" << v.side << ", " <<
        ".price=" << v.price << ", " <<
        ".remaining_size=" << v.remaining_size << ", " <<
        ".reason=" << v.reason <<
        "}";
}

Full::Done tag_invoke(boost::json::value_to_tag<Full::Done>, boost::json::value const& src) {
    auto obj = src.as_object();

    return Full::Done{
        .time = boost::json::value_to<std::string>(obj.at("time")),
        .product_id = boost::json::value_to<std::string>(obj.at("product_id")),
        .sequence = obj.at("sequence").as_int64(),
        .order_id = boost::json::value_to<std::string>(obj.at("order_id")),
        .side = boost::json::value_to<std::string>(obj.at("side")),
        .price = decimal_from_key(obj, "price"),
        .remaining_size = decimal_from_key(obj, "remaining_size"),
        .reason = boost::json::value_to<std::string>(obj.at("reason")),
    };
}

std::ostream& operator<<(std::ostream& os, const Full::Match& v) {
    return os << std::setprecision(std::numeric_limits<Decimal>::max_digits10) << "{" <<
        ".time=" << v.time << ", " <<
        ".product_id=" << v.product_id << ", " <<
        ".sequence=" << v.sequence << ", " <<
        ".maker_order_id=" << v.maker_order_id << ", " <<
        ".taker_order_id=" << v.taker_order_id << ", " <<
        ".side=" << v.side << ", " <<
        ".price=" << v.price << ", " <<
        ".size=" << v.size <<
        "}";
}

Full::Match tag_invoke(boost::json::value_to_tag<Full::Match>, boost::json::value const& src) {
    return Full::Match{
        .time = boost::json::value_to<std::string>(src.at("time")),
        .product_id = boost::json::value_to<std::string>(src.at("product_id")),
        .sequence = src.at("sequence").as_int64(),
        .maker_order_id = boost::json::value_to<std::string>(src.at("maker_order_id")),
        .taker_order_id = boost::json::value_to<std::string>(src.at("taker_order_id")),
        .side = boost::json::value_to<std::string>(src.at("side")),
        .price = Decimal{boost::json::value_to<std::string>(src.at("price"))},
        .size = Decimal{boost::json::value_to<std::string>(src.at("size"))},
    };
}

std::ostream& operator<<(std::ostream& os, const Full::Change& v) {
    return os << std::setprecision(std::numeric_limits<Decimal>::max_digits10) << "{" <<
        ".time=" << v.time << ", " <<
        ".product_id=" << v.product_id << ", " <<
        ".sequence=" << v.sequence << ", " <<
        ".order_id=" << v.order_id << ", " <<
        ".side=" << v.side << ", " <<
        ".price=" << v.price << ", " <<
        ".old_size=" << v.old_size << ", " <<
        ".new_size=" << v.new_size << ", " <<       
        "}";
}

Full::Change tag_invoke(boost::json::value_to_tag<Full::Change>, boost::json::value const& src) {
    auto obj = src.as_object();

    return Full::Change{
        .time = boost::json::value_to<std::string>(obj.at("time")),
        .product_id = boost::json::value_to<std::string>(obj.at("product_id")),
        .sequence = obj.at("sequence").as_int64(),
        .order_id = boost::json::value_to<std::string>(obj.at("order_id")),
        .side = boost::json::value_to<std::string>(obj.at("side")),
        .price = decimal_from_key(obj, "price"),
        .old_size = decimal_from_key(obj, "old_size"),
        .new_size = decimal_from_key(obj, "new_size"),
    };
}

Full::Activate tag_invoke(boost::json::value_to_tag<Full::Activate>, boost::json::value const& src) {
    return Full::Activate{};
}

} // namespace coinbase

#endif