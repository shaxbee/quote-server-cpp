#include "full.h"

#include <unordered_map>

#include <boost/json.hpp>

namespace coinbase {

namespace {

Decimal decimal_from_key(const boost::json::object& obj, const char* key) {
    if (obj.contains(key)) {
        return Decimal{boost::json::value_to<std::string>(obj.at(key))};
    }

    return Decimal{};
};

static std::unordered_map<std::string, Full::Type> full_type_names = {
    {"received", Full::Type::Received},
    {"open", Full::Type::Open},
    {"done", Full::Type::Done},
    {"match", Full::Type::Match},
    {"change", Full::Type::Change},
    {"activate", Full::Type::Activate},
};

} // anonymous namespace

void FullVisitor::apply(const Full& full) {
    std::visit([&](const auto& v) { visit(full, v); }, full.payload);
};

Full parse_full(std::string data) {
    return boost::json::value_to<Full>(boost::json::parse(data));
};

Full tag_invoke(boost::json::value_to_tag<Full>, boost::json::value const& src) {
    auto type = boost::json::value_to<Full::Type>(src.at("type"));

    Full::Payload payload;
    switch (type) {
    case (Full::Type::Received):
        payload = boost::json::value_to<Received>(src); break;
    case Full::Type::Open:
        payload = boost::json::value_to<Open>(src); break;
    case Full::Type::Done:
        payload = boost::json::value_to<Done>(src); break;
    case Full::Type::Match:
        payload = boost::json::value_to<Match>(src); break;
    case Full::Type::Change:
        payload = boost::json::value_to<Change>(src); break;
    case Full::Type::Activate:
        payload = boost::json::value_to<Activate>(src); break;
    }

    return {
        .type = type,
        .time = boost::json::value_to<std::string>(src.at("time")),
        .product_id = boost::json::value_to<std::string>(src.at("product_id")),
        .sequence = src.at("sequence").as_int64(),
        .side = boost::json::value_to<std::string>(src.at("side")),
        .payload = payload,
    };
}

Full::Type tag_invoke(boost::json::value_to_tag<Full::Type>, boost::json::value const& src) {
    auto it = full_type_names.find(boost::json::value_to<std::string>(src));
    if (it == full_type_names.end()) {
        throw std::invalid_argument("unsupported Full::Type value");
    };

    return it->second;
}

Received tag_invoke(boost::json::value_to_tag<Received>, boost::json::value const& src) {
    auto obj = src.as_object();
    
    return {
        .order_id = boost::json::value_to<std::string>(obj.at("order_id")),
        .order_type = boost::json::value_to<std::string>(obj.at("order_type")),
        .size = decimal_from_key(obj, "size"),
        .price = decimal_from_key(obj, "price"),
        .funds = decimal_from_key(obj, "funds")
    };
}

Open tag_invoke(boost::json::value_to_tag<Open>, boost::json::value const& src) {
    return Open{
        .order_id = boost::json::value_to<std::string>(src.at("order_id")),
        .price = Decimal{boost::json::value_to<std::string>(src.at("price"))},
        .remaining_size = Decimal{boost::json::value_to<std::string>(src.at("remaining_size"))}
    };
}

Done tag_invoke(boost::json::value_to_tag<Done>, boost::json::value const& src) {
    auto obj = src.as_object();

    return {
        .order_id = boost::json::value_to<std::string>(obj.at("order_id")),
        .price = decimal_from_key(obj, "price"),
        .remaining_size = decimal_from_key(obj, "remaining_size"),
        .reason = boost::json::value_to<std::string>(obj.at("reason")),
    };
}

Match tag_invoke(boost::json::value_to_tag<Match>, boost::json::value const& src) {
    return {
        .maker_order_id = boost::json::value_to<std::string>(src.at("maker_order_id")),
        .taker_order_id = boost::json::value_to<std::string>(src.at("taker_order_id")),
        .price = Decimal{boost::json::value_to<std::string>(src.at("price"))},
        .size = Decimal{boost::json::value_to<std::string>(src.at("size"))},
    };
}

Change tag_invoke(boost::json::value_to_tag<Change>, boost::json::value const& src) {
    auto obj = src.as_object();

    return {
        .order_id = boost::json::value_to<std::string>(obj.at("order_id")),
        .price = decimal_from_key(obj, "price"),
        .old_size = decimal_from_key(obj, "old_size"),
        .new_size = decimal_from_key(obj, "new_size"),
    };
}

Activate tag_invoke(boost::json::value_to_tag<Activate>, boost::json::value const& src) {
    return Activate{};
}

std::ostream& operator<<(std::ostream& os, const Full& v) {
    os << "{" <<
        ".type=" << v.type << ", " <<
        ".time=" << v.time << ", " <<
        ".product_id=" << v.product_id << ", " <<
        ".sequence=" << v.sequence << ", " <<
        ".side=" << v.side << ", ";

    std::visit([&os](const auto& payload) {
        os << ".payload=" << payload;
    }, v.payload);

    os << "}";

    return os;
}

std::ostream& operator<<(std::ostream& os, const Full::Type& v) {
    switch (v) {
    case (Full::Type::Received):
        os << "received"; break;
    case Full::Type::Open:
        os << "open"; break;
    case Full::Type::Done:
        os << "done"; break;
    case Full::Type::Match:
        os << "match"; break;
    case Full::Type::Change:
        os << "change"; break;
    case Full::Type::Activate:
        os << "activate"; break;
    };

    return os;
}

std::ostream& operator<<(std::ostream& os, const Received& v) {
    return os << std::setprecision(std::numeric_limits<Decimal>::max_digits10) << "{" <<
        ".order_id=" << v.order_id << ", " <<
        ".order_type=" << v.order_type << ", " <<
        ".size=" << v.size << ", " <<
        ".price=" << v.price << ", " <<
        ".funds=" << v.funds <<
        "}";
}

std::ostream& operator<<(std::ostream& os, const Open& v) {
    return os << std::setprecision(std::numeric_limits<Decimal>::max_digits10) << "{" <<
        ".order_id=" << v.order_id << ", " <<
        ".price=" << v.price << ", " <<
        ".remaining_size=" << v.remaining_size <<
        "}";
}

std::ostream& operator<<(std::ostream& os, const Done& v) {
    return os << std::setprecision(std::numeric_limits<Decimal>::max_digits10) << "{" <<
        ".order_id=" << v.order_id << ", " <<
        ".price=" << v.price << ", " <<
        ".remaining_size=" << v.remaining_size << ", " <<
        ".reason=" << v.reason <<
        "}";
}

std::ostream& operator<<(std::ostream& os, const Match& v) {
    return os << std::setprecision(std::numeric_limits<Decimal>::max_digits10) << "{" <<
        ".maker_order_id=" << v.maker_order_id << ", " <<
        ".taker_order_id=" << v.taker_order_id << ", " <<
        ".price=" << v.price << ", " <<
        ".size=" << v.size <<
        "}";
}

std::ostream& operator<<(std::ostream& os, const Change& v) {
    return os << std::setprecision(std::numeric_limits<Decimal>::max_digits10) << "{" <<
        ".order_id=" << v.order_id << ", " <<
        ".price=" << v.price << ", " <<
        ".old_size=" << v.old_size << ", " <<
        ".new_size=" << v.new_size << ", " <<       
        "}";
}

std::ostream& operator<<(std::ostream& os, const Activate& v) {
    return os << "{}";
}

} // namespace coinbase