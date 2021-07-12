#ifndef SERVER_TRADE_H
#define SERVER_TRADE_H 1

struct Trade {
    std::string product_id;
    std::string time;
    Side side;
    std::string maker_order_id;
    std::string taker_order_id;
    Decimal price;
    Decimal size;
};

#endif