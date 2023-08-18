//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_TEST_IMPL_ERROR_HPP
#define BOOST_BEAST_TEST_IMPL_ERROR_HPP

#include <beast/beast/core/error.hpp>
#include <beast/beast/core/string.hpp>
#include <type_traits>

#ifdef ASIO_STANDALONE
namespace std {
#else
namespace bnet {
namespace system {
#endif
template<>
struct is_error_code_enum<
    bnet::beast::test::error>
        : std::true_type
{
};
#ifdef ASIO_STANDALONE
} // std
#else
} // system
} // boost
#endif

namespace bnet {
namespace beast {
namespace test {

BOOST_BEAST_DECL
error_code
make_error_code(error e) noexcept;

} // test
} // beast
} // boost

#endif
