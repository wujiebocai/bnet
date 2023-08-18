//
// Copyright (c) 2015-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_CORE_IMPL_FILE_WIN32_IPP
#define BOOST_BEAST_CORE_IMPL_FILE_WIN32_IPP

#include <beast/beast/core/file_win32.hpp>

#if BOOST_BEAST_USE_WIN32_FILE

#include <beast/beast/core/detail/win32_unicode_path.hpp>
#include <beast/core/exchange.hpp>
#include <beast/winapi/access_rights.hpp>
#include <beast/winapi/error_codes.hpp>
#include <beast/winapi/get_last_error.hpp>
#include <limits>
#include <utility>

namespace bnet {
namespace beast {

namespace detail {

// VFALCO Can't seem to get boost/detail/winapi to work with
//        this so use the non-Ex version for now.
BOOST_BEAST_DECL
bnet::winapi::BOOL_
set_file_pointer_ex(
    bnet::winapi::HANDLE_ hFile,
    bnet::winapi::LARGE_INTEGER_ lpDistanceToMove,
    bnet::winapi::PLARGE_INTEGER_ lpNewFilePointer,
    bnet::winapi::DWORD_ dwMoveMethod)
{
    auto dwHighPart = lpDistanceToMove.u.HighPart;
    auto dwLowPart = bnet::winapi::SetFilePointer(
        hFile,
        lpDistanceToMove.u.LowPart,
        &dwHighPart,
        dwMoveMethod);
    if(dwLowPart == bnet::winapi::INVALID_SET_FILE_POINTER_)
        return 0;
    if(lpNewFilePointer)
    {
        lpNewFilePointer->u.LowPart = dwLowPart;
        lpNewFilePointer->u.HighPart = dwHighPart;
    }
    return 1;
}

} // detail

file_win32::
~file_win32()
{
    if(h_ != bnet::winapi::INVALID_HANDLE_VALUE_)
        bnet::winapi::CloseHandle(h_);
}

file_win32::
file_win32(file_win32&& other)
    : h_(bnet::exchange(other.h_,
        bnet::winapi::INVALID_HANDLE_VALUE_))
{
}

file_win32&
file_win32::
operator=(file_win32&& other)
{
    if(&other == this)
        return *this;
    if(h_)
        bnet::winapi::CloseHandle(h_);
    h_ = other.h_;
    other.h_ = bnet::winapi::INVALID_HANDLE_VALUE_;
    return *this;
}

void
file_win32::
native_handle(native_handle_type h)
{
     if(h_ != bnet::winapi::INVALID_HANDLE_VALUE_)
        bnet::winapi::CloseHandle(h_);
    h_ = h;
}

void
file_win32::
close(error_code& ec)
{
    if(h_ != bnet::winapi::INVALID_HANDLE_VALUE_)
    {
        if(! bnet::winapi::CloseHandle(h_))
            ec.assign(bnet::winapi::GetLastError(),
                system_category());
        else
            ec = {};
        h_ = bnet::winapi::INVALID_HANDLE_VALUE_;
    }
    else
    {
        ec = {};
    }
}

void
file_win32::
open(char const* path, file_mode mode, error_code& ec)
{
    if(h_ != bnet::winapi::INVALID_HANDLE_VALUE_)
    {
        bnet::winapi::CloseHandle(h_);
        h_ = bnet::winapi::INVALID_HANDLE_VALUE_;
    }
    bnet::winapi::DWORD_ share_mode = 0;
    bnet::winapi::DWORD_ desired_access = 0;
    bnet::winapi::DWORD_ creation_disposition = 0;
    bnet::winapi::DWORD_ flags_and_attributes = 0;
/*
                             |                    When the file...
    This argument:           |             Exists            Does not exist
    -------------------------+------------------------------------------------------
    CREATE_ALWAYS            |            Truncates             Creates
    CREATE_NEW         +-----------+        Fails               Creates
    OPEN_ALWAYS     ===| does this |===>    Opens               Creates
    OPEN_EXISTING      +-----------+        Opens                Fails
    TRUNCATE_EXISTING        |            Truncates              Fails
*/
    switch(mode)
    {
    default:
    case file_mode::read:
        desired_access = bnet::winapi::GENERIC_READ_;
        share_mode = bnet::winapi::FILE_SHARE_READ_;
        creation_disposition = bnet::winapi::OPEN_EXISTING_;
        flags_and_attributes = 0x10000000; // FILE_FLAG_RANDOM_ACCESS
        break;

    case file_mode::scan:           
        desired_access = bnet::winapi::GENERIC_READ_;
        share_mode = bnet::winapi::FILE_SHARE_READ_;
        creation_disposition = bnet::winapi::OPEN_EXISTING_;
        flags_and_attributes = 0x08000000; // FILE_FLAG_SEQUENTIAL_SCAN
        break;

    case file_mode::write:          
        desired_access = bnet::winapi::GENERIC_READ_ |
                         bnet::winapi::GENERIC_WRITE_;
        creation_disposition = bnet::winapi::CREATE_ALWAYS_;
        flags_and_attributes = 0x10000000; // FILE_FLAG_RANDOM_ACCESS
        break;

    case file_mode::write_new:      
        desired_access = bnet::winapi::GENERIC_READ_ |
                         bnet::winapi::GENERIC_WRITE_;
        creation_disposition = bnet::winapi::CREATE_NEW_;
        flags_and_attributes = 0x10000000; // FILE_FLAG_RANDOM_ACCESS
        break;

    case file_mode::write_existing: 
        desired_access = bnet::winapi::GENERIC_READ_ |
                         bnet::winapi::GENERIC_WRITE_;
        creation_disposition = bnet::winapi::OPEN_EXISTING_;
        flags_and_attributes = 0x10000000; // FILE_FLAG_RANDOM_ACCESS
        break;

    case file_mode::append:         
        desired_access = bnet::winapi::GENERIC_READ_ |
                         bnet::winapi::GENERIC_WRITE_;

        creation_disposition = bnet::winapi::OPEN_ALWAYS_;
        flags_and_attributes = 0x08000000; // FILE_FLAG_SEQUENTIAL_SCAN
        break;

    case file_mode::append_existing:
        desired_access = bnet::winapi::GENERIC_READ_ |
                         bnet::winapi::GENERIC_WRITE_;
        creation_disposition = bnet::winapi::OPEN_EXISTING_;
        flags_and_attributes = 0x08000000; // FILE_FLAG_SEQUENTIAL_SCAN
        break;
    }
    
    detail::win32_unicode_path unicode_path(path, ec);
    if (ec)
        return;
    h_ = ::CreateFileW(
        unicode_path.c_str(),
        desired_access,
        share_mode,
        NULL,
        creation_disposition,
        flags_and_attributes,
        NULL);
    if (h_ == bnet::winapi::INVALID_HANDLE_VALUE_)
    {
        ec.assign(bnet::winapi::GetLastError(),
            system_category());
        return;
    }
    if (mode == file_mode::append ||
        mode == file_mode::append_existing)
    {
        bnet::winapi::LARGE_INTEGER_ in;
        in.QuadPart = 0;
        if (!detail::set_file_pointer_ex(h_, in, 0,
            bnet::winapi::FILE_END_))
        {
            ec.assign(bnet::winapi::GetLastError(),
                system_category());
            bnet::winapi::CloseHandle(h_);
            h_ = bnet::winapi::INVALID_HANDLE_VALUE_;
            return;
        }
    }
    ec = {};
}

std::uint64_t
file_win32::
size(error_code& ec) const
{
    if(h_ == bnet::winapi::INVALID_HANDLE_VALUE_)
    {
        ec = make_error_code(errc::bad_file_descriptor);
        return 0;
    }
    bnet::winapi::LARGE_INTEGER_ fileSize;
    if(! bnet::winapi::GetFileSizeEx(h_, &fileSize))
    {
        ec.assign(bnet::winapi::GetLastError(),
            system_category());
        return 0;
    }
    ec = {};
    return fileSize.QuadPart;
}

std::uint64_t
file_win32::
pos(error_code& ec)
{
    if(h_ == bnet::winapi::INVALID_HANDLE_VALUE_)
    {
        ec = make_error_code(errc::bad_file_descriptor);
        return 0;
    }
    bnet::winapi::LARGE_INTEGER_ in;
    bnet::winapi::LARGE_INTEGER_ out;
    in.QuadPart = 0;
    if(! detail::set_file_pointer_ex(h_, in, &out,
        bnet::winapi::FILE_CURRENT_))
    {
        ec.assign(bnet::winapi::GetLastError(),
            system_category());
        return 0;
    }
    ec = {};
    return out.QuadPart;
}

void
file_win32::
seek(std::uint64_t offset, error_code& ec)
{
    if(h_ == bnet::winapi::INVALID_HANDLE_VALUE_)
    {
        ec = make_error_code(errc::bad_file_descriptor);
        return;
    }
    bnet::winapi::LARGE_INTEGER_ in;
    in.QuadPart = offset;
    if(! detail::set_file_pointer_ex(h_, in, 0,
        bnet::winapi::FILE_BEGIN_))
    {
        ec.assign(bnet::winapi::GetLastError(),
            system_category());
        return;
    }
    ec = {};
}

std::size_t
file_win32::
read(void* buffer, std::size_t n, error_code& ec)
{
    if(h_ == bnet::winapi::INVALID_HANDLE_VALUE_)
    {
        ec = make_error_code(errc::bad_file_descriptor);
        return 0;
    }
    std::size_t nread = 0;
    while(n > 0)
    {
        bnet::winapi::DWORD_ amount;
        if(n > (std::numeric_limits<
                bnet::winapi::DWORD_>::max)())
            amount = (std::numeric_limits<
                bnet::winapi::DWORD_>::max)();
        else
            amount = static_cast<
                bnet::winapi::DWORD_>(n);
        bnet::winapi::DWORD_ bytesRead;
        if(! ::ReadFile(h_, buffer, amount, &bytesRead, 0))
        {
            auto const dwError = bnet::winapi::GetLastError();
            if(dwError != bnet::winapi::ERROR_HANDLE_EOF_)
                ec.assign(dwError, system_category());
            else
                ec = {};
            return nread;
        }
        if(bytesRead == 0)
            return nread;
        n -= bytesRead;
        nread += bytesRead;
        buffer = static_cast<char*>(buffer) + bytesRead;
    }
    ec = {};
    return nread;
}

std::size_t
file_win32::
write(void const* buffer, std::size_t n, error_code& ec)
{
    if(h_ == bnet::winapi::INVALID_HANDLE_VALUE_)
    {
        ec = make_error_code(errc::bad_file_descriptor);
        return 0;
    }
    std::size_t nwritten = 0;
    while(n > 0)
    {
        bnet::winapi::DWORD_ amount;
        if(n > (std::numeric_limits<
                bnet::winapi::DWORD_>::max)())
            amount = (std::numeric_limits<
                bnet::winapi::DWORD_>::max)();
        else
            amount = static_cast<
                bnet::winapi::DWORD_>(n);
        bnet::winapi::DWORD_ bytesWritten;
        if(! ::WriteFile(h_, buffer, amount, &bytesWritten, 0))
        {
            auto const dwError = bnet::winapi::GetLastError();
            if(dwError != bnet::winapi::ERROR_HANDLE_EOF_)
                ec.assign(dwError, system_category());
            else
                ec = {};
            return nwritten;
        }
        if(bytesWritten == 0)
            return nwritten;
        n -= bytesWritten;
        nwritten += bytesWritten;
        buffer = static_cast<char const*>(buffer) + bytesWritten;
    }
    ec = {};
    return nwritten;
}

} // beast
} // boost

#endif

#endif
