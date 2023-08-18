#ifndef BOOST_CORE_DETAIL_SPLITMIX64_HPP_INCLUDED
#define BOOST_CORE_DETAIL_SPLITMIX64_HPP_INCLUDED

// Copyright 2020 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// An implementation of splitmix64 for testing purposes,
// derived from Sebastiano Vigna's public domain implementation
// http://xorshift.di.unimi.it/splitmix64.c

#include <beast/cstdint.hpp>

namespace bnet
{
namespace detail
{

class splitmix64
{
private:

    bnet::uint64_t x_;

public:

    splitmix64(): x_( 0 )
    {
    }

    explicit splitmix64( bnet::uint64_t seed ): x_( seed )
    {
    }

    bnet::uint64_t operator()()
    {
        x_ += ( bnet::uint64_t(0x9e3779b9u) << 32 ) + 0x7f4a7c15u;

        bnet::uint64_t z = x_;

        z ^= z >> 30;
        z *= ( bnet::uint64_t(0xbf58476du) << 32 ) + 0x1ce4e5b9u;
        z ^= z >> 27;
        z *= ( bnet::uint64_t(0x94d049bbu) << 32 ) + 0x133111ebu;
        z ^= z >> 31;

        return z;
    }
};

} // namespace detail
} // namespace bnet

#endif // #ifndef BOOST_CORE_DETAIL_SPLITMIX64_HPP_INCLUDED
