#ifndef COINBASE_FULL_H
#define COINBASE_FULL_H 1

#include <any>
#include <cstdint>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <variant>

#include "decimal.h"
#include "time.h"

namespace coinbase {

struct Received {
    std::string order_id;
    std::string order_type;
    Decimal size;
    Decimal price;
    Decimal funds;

    bool operator==(const Received&) const = default;
};

struct Open {
    std::string order_id;
    Decimal price;
    Decimal remaining_size;

    bool operator==(const Open&) const = default;
};

struct Done {
    std::string order_id;
    Decimal price;
    Decimal remaining_size;
    std::string reason;

    bool operator==(const Done&) const = default;
};

struct Match {
    std::string maker_order_id;
    std::string taker_order_id;
    Decimal price;
    Decimal size;

    bool operator==(const Match&) const = default;
};

struct Change {
    std::string order_id;
    Decimal price;
    Decimal old_size;
    Decimal new_size;
    Decimal old_funds;
    Decimal new_funds;

    bool operator==(const Change&) const = default;
};

struct Activate {
    bool operator==(const Activate&) const = default;
};

struct Full {
    enum class Type: int {
        Received,
        Open,
        Done,
        Match,
        Change,
        Activate
    };

    using Payload = std::variant<Received, Open, Done, Match, Change, Activate>;

    Type type;
    std::string time;
    std::string product_id;
    std::int64_t sequence;
    std::string side;

    Payload payload;

    bool operator==(const Full&) const = default;
};

struct FullVisitor {
    void apply(const Full&);

    virtual void visit(const Full&, const Received&) {};
    virtual void visit(const Full&, const Open&) {};
    virtual void visit(const Full&, const Done&) {};
    virtual void visit(const Full&, const Match&) {};
    virtual void visit(const Full&, const Change&) {};
    virtual void visit(const Full&, const Activate&) {};
};

Full parse_full(std::string data);

std::ostream& operator<<(std::ostream& os, const Full& v);
std::ostream& operator<<(std::ostream& os, const Full::Type& v);
std::ostream& operator<<(std::ostream& os, const Received& v);
std::ostream& operator<<(std::ostream& os, const Open& v);
std::ostream& operator<<(std::ostream& os, const Done& v);
std::ostream& operator<<(std::ostream& os, const Match& v);
std::ostream& operator<<(std::ostream& os, const Change& v);
std::ostream& operator<<(std::ostream& os, const Activate& v);

} // namespace coinbase

#endif