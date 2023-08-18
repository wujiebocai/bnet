#ifndef BOOST_CORE_DETAIL_SP_THREAD_YIELD_HPP_INCLUDED
#define BOOST_CORE_DETAIL_SP_THREAD_YIELD_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// boost/core/detail/sp_thread_yield.hpp
//
// inline void bost::core::sp_thread_yield();
//
//   Gives up the remainder of the time slice,
//   as if by calling sched_yield().
//
// Copyright 2008, 2020 Peter Dimov
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt

#include <beast/config.hpp>
#include <beast/config/pragma_message.hpp>

#if defined( _WIN32 ) || defined( __WIN32__ ) || defined( __CYGWIN__ )

#if defined(BOOST_SP_REPORT_IMPLEMENTATION)
  BOOST_PRAGMA_MESSAGE("Using SwitchToThread() in sp_thread_yield")
#endif

#include <beast/core/detail/sp_win32_sleep.hpp>

namespace bnet
{
namespace core
{
namespace detail
{

inline void sp_thread_yield() BOOST_NOEXCEPT
{
    SwitchToThread();
}

} // namespace detail

using bnet::core::detail::sp_thread_yield;

} // namespace core
} // namespace bnet

#elif defined(BOOST_HAS_SCHED_YIELD)

#if defined(BOOST_SP_REPORT_IMPLEMENTATION)
  BOOST_PRAGMA_MESSAGE("Using sched_yield() in sp_thread_yield")
#endif

#ifndef _AIX
# include <sched.h>
#else
  // AIX's sched.h defines ::var which sometimes conflicts with Lambda's var
  extern "C" int sched_yield(void);
#endif

namespace bnet
{
namespace core
{

inline void sp_thread_yield() BOOST_NOEXCEPT
{
    sched_yield();
}

} // namespace core
} // namespace bnet

#else

#if defined(BOOST_SP_REPORT_IMPLEMENTATION)
  BOOST_PRAGMA_MESSAGE("Using sp_thread_pause() in sp_thread_yield")
#endif

#include <beast/core/detail/sp_thread_pause.hpp>

namespace bnet
{
namespace core
{

inline void sp_thread_yield() BOOST_NOEXCEPT
{
    sp_thread_pause();
}

} // namespace core
} // namespace bnet

#endif

#endif // #ifndef BOOST_CORE_DETAIL_SP_THREAD_YIELD_HPP_INCLUDED
