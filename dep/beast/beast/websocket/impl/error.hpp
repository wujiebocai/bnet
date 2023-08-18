//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_WEBSOCKET_IMPL_ERROR_HPP
#define BOOST_BEAST_WEBSOCKET_IMPL_ERROR_HPP

#ifdef ASIO_STANDALONE
namespace std {
#else
namespace bnet {
namespace system {
#endif
template<>
struct is_error_code_enum<::bnet::beast::websocket::error>
{
    static bool const value = true;
};
template<>
struct is_error_condition_enum<::bnet::beast::websocket::condition>
{
    static bool const value = true;
};
#ifdef ASIO_STANDALONE
} // std
#else
} // system
} // boost
#endif

namespace bnet {
namespace beast {
namespace websocket {

BOOST_BEAST_DECL
error_code
make_error_code(error e);

BOOST_BEAST_DECL
error_condition
make_error_condition(condition c);

} // websocket
} // beast
} // boost

#endif
