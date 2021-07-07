#ifndef COINBASE_DECIMAL_H
#define COINBASE_DECIMAL_H 1

#include <boost/json.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>

using Decimal = boost::multiprecision::number<boost::multiprecision::cpp_dec_float<8>>;

#endif