#ifndef COINBASE_DECIMAL_H
#define COINBASE_DECIMAL_H 1

#include <boost/multiprecision/cpp_dec_float.hpp>

namespace coinbase {

using Decimal = boost::multiprecision::number<boost::multiprecision::cpp_dec_float<8>>;

}; // namespace coinbase

#endif