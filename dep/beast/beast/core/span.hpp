//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_CORE_SPAN_HPP
#define BOOST_BEAST_CORE_SPAN_HPP

#include <beast/beast/core/detail/config.hpp>
#include <beast/core/span.hpp>

namespace bnet {
namespace beast {

template<class T, std::size_t E = bnet::dynamic_extent>
using span = bnet::span<T, E>;

} // beast
} // boost

#endif
