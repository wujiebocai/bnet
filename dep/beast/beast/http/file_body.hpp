//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_HTTP_FILE_BODY_HPP
#define BOOST_BEAST_HTTP_FILE_BODY_HPP

#include <beast/beast/core/file.hpp>
#include <beast/beast/http/basic_file_body.hpp>
#include <beast/assert.hpp>
#include <optional>
#include <algorithm>
#include <cstdio>
#include <cstdint>
#include <utility>

namespace bnet {
namespace beast {
namespace http {

/// A message body represented by a file on the filesystem.
using file_body = basic_file_body<file>;

} // http
} // beast
} // boost

#ifndef BOOST_BEAST_NO_FILE_BODY_WIN32
#include <beast/beast/http/impl/file_body_win32.hpp>
#endif

#endif
