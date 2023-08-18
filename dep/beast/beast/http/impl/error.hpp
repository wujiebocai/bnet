//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_HTTP_IMPL_ERROR_HPP
#define BOOST_BEAST_HTTP_IMPL_ERROR_HPP

#include <type_traits>
#include <system_error>

#ifdef ASIO_STANDALONE
namespace std {
#else
namespace bnet {
namespace system {
#endif
template<>
struct is_error_code_enum<::bnet::beast::http::error>
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
namespace http {

BOOST_BEAST_DECL
error_code
make_error_code(error ev);

} // http
} // beast
} // boost

#endif
