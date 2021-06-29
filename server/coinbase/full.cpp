#include "full.h"

#include <stdexcept>
#include <unordered_map>

using namespace boost::json;

namespace coinbase {

namespace {
    static std::unordered_map<std::string, Full::Type> full_type_names = {
        {"received", Full::Type::Received},
        {"open", Full::Type::Open},
        {"done", Full::Type::Done},
        {"match", Full::Type::Match},
        {"change", Full::Type::Change},
        {"activate", Full::Type::Activate},
    };
}

Full tag_invoke(value_to_tag<Full>, value const& src) {
    auto type = value_to<Full::Type>(src.at("type"));
    std::any value;

    switch (type) {
    case Full::Type::Received:
        value = value_to<Full::Received>(src);
        break;
    case Full::Type::Open:
        value = value_to<Full::Open>(src);
        break;
    case Full::Type::Done:
        value = value_to<Full::Done>(src);
        break;
    case Full::Type::Match:
        value = value_to<Full::Match>(src);
        break;
    case Full::Type::Change:
        value = value_to<Full::Change>(src);
        break;
    case Full::Type::Activate:
        value = value_to<Full::Activate>(src);
        break;
    }

    return Full{
        .type = type,
        .time = value_to<std::string>(src.at("time")),
        .product_id = value_to<std::string>(src.at("product_id")),
        .sequence = src.at("sequence").as_int64(),
        .side = value_to<std::string>(src.at("side")),
        .value = value,
    };
}

Full::Type tag_invoke(value_to_tag<Full::Type>, value const& src) {
    auto it = full_type_names.find(value_to<std::string>(src));
    if (it == full_type_names.end()) {
        throw std::invalid_argument("unsupported Full::Type value");
    };

    return it->second;
}

Full::Received tag_invoke(value_to_tag<Full::Received>, value const& src) {
    return Full::Received{};
}

Full::Open tag_invoke(value_to_tag<Full::Open>, value const& src) {
    return Full::Open{
        .order_id = value_to<std::string>(src.at("order_id")),
        .price = Decimal{value_to<std::string>(src.at("price"))},
        .remaining_size = Decimal{value_to<std::string>(src.at("remaining_size"))},
    };
}

Full::Done tag_invoke(value_to_tag<Full::Done>, value const& src) {
    return Full::Done{
        .order_id = value_to<std::string>(src.at("order_id")),
        .price = Decimal{value_to<std::string>(src.at("price"))},
        .remaining_size = Decimal{value_to<std::string>(src.at("remaining_size"))},
        .reason = value_to<std::string>(src.at("reason")),
    };
}

Full::Match tag_invoke(value_to_tag<Full::Match>, value const& src) {
    return Full::Match{
        .maker_order_id = value_to<std::string>(src.at("maker_order_id")),
        .taker_order_id = value_to<std::string>(src.at("taker_order_id")),
        .price = Decimal{value_to<std::string>(src.at("price"))},
        .size = Decimal{value_to<std::string>(src.at("size"))},
    };
}

Full::Change tag_invoke(value_to_tag<Full::Change>, value const& src) {
    return Full::Change{
        .order_id = value_to<std::string>(src.at("order_id")),
        .price = Decimal{value_to<std::string>(src.at("price"))},
        .old_size = Decimal{value_to<std::string>(src.at("old_size"))},
        .new_size = Decimal{value_to<std::string>(src.at("new_size"))},
    };
}

Full::Activate tag_invoke(value_to_tag<Full::Activate>, value const& src) {
    return Full::Activate{};
}


} // namespace coinbase