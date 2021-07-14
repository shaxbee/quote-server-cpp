#include "quote_service.h"

#include <catch2/catch.hpp>

TEST_CASE( "QuoteService", "[quote_service]" ) {
    CoinbaseSource source{logger, client, config.products};
    QuoteServiceImpl service(source);
}