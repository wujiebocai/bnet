#ifndef BOOST_ENDIAN_DETAIL_INTEGRAL_BY_SIZE_HPP_INCLUDED
#define BOOST_ENDIAN_DETAIL_INTEGRAL_BY_SIZE_HPP_INCLUDED

// Copyright 2019 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#include <beast/cstdint.hpp>
#include <beast/config.hpp>
#include <cstddef>

namespace bnet
{
namespace endian
{
namespace detail
{

template<std::size_t N> struct integral_by_size
{
};

template<> struct integral_by_size<1>
{
    typedef uint8_t type;
};

template<> struct integral_by_size<2>
{
    typedef uint16_t type;
};

template<> struct integral_by_size<4>
{
    typedef uint32_t type;
};

template<> struct integral_by_size<8>
{
    typedef uint64_t type;
};

#if defined(BOOST_HAS_INT128)

template<> struct integral_by_size<16>
{
    typedef uint128_type type;
};

#endif

} // namespace detail
} // namespace endian
} // namespace bnet

#endif  // BOOST_ENDIAN_DETAIL_INTEGRAL_BY_SIZE_HPP_INCLUDED
