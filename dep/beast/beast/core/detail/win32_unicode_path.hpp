//
// Copyright (c) 2019 Mika Fischer (mika.fischer@zoopnet.de)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_CORE_DETAIL_WIN32_UNICODE_PATH_HPP
#define BOOST_BEAST_CORE_DETAIL_WIN32_UNICODE_PATH_HPP

#ifdef _WIN32
#include <beast/config.hpp>
#include <beast/beast/core/error.hpp>
#include <beast/winapi/character_code_conversion.hpp>
#include <beast/winapi/file_management.hpp>
#include <beast/winapi/get_last_error.hpp>
#include <array>
#include <vector>

namespace bnet {
namespace beast {
namespace detail {

class win32_unicode_path
{
    using WCHAR_ = bnet::winapi::WCHAR_;

public:
    win32_unicode_path(const char* utf8_path, error_code& ec) {
        int ret = mb2wide(utf8_path, static_buf_.data(),
            static_buf_.size());
        if (ret == 0)
        {
            int sz = mb2wide(utf8_path, nullptr, 0);
            if (sz == 0)
            {
                ec.assign(bnet::winapi::GetLastError(),
                    system_category());
                return;
            }
            dynamic_buf_.resize(sz);
            int ret2 = mb2wide(utf8_path,
                dynamic_buf_.data(),
                dynamic_buf_.size());
            if (ret2 == 0)
            {
                ec.assign(bnet::winapi::GetLastError(),
                    system_category());
                return;
            }
        }
    }

    WCHAR_ const* c_str() const noexcept
    {
        return dynamic_buf_.empty()
            ? static_buf_.data()
            : dynamic_buf_.data();
    }

private:
    int mb2wide(const char* utf8_path, WCHAR_* buf, size_t sz)
    {
        return bnet::winapi::MultiByteToWideChar(
            bnet::winapi::CP_UTF8_,
            bnet::winapi::MB_ERR_INVALID_CHARS_,
            utf8_path, -1,
            buf, static_cast<int>(sz));
    }

    std::array<WCHAR_, bnet::winapi::MAX_PATH_> static_buf_;
    std::vector<WCHAR_> dynamic_buf_;
};

} // detail
} // beast
} // boost
#endif

#endif
