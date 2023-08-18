//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_DETAIL_BIND_HANDLER_HPP
#define BOOST_BEAST_DETAIL_BIND_HANDLER_HPP

#include <beast/beast/core/error.hpp>
#include <beast/beast/core/detail/tuple.hpp>
#include <asio/associated_allocator.hpp>
#include <asio/associated_cancellation_slot.hpp>
#include <asio/associated_executor.hpp>
#include <asio/handler_alloc_hook.hpp>
#include <asio/handler_continuation_hook.hpp>
#include <asio/handler_invoke_hook.hpp>
#include <beast/core/ignore_unused.hpp>
#include <beast/mp11/integer_sequence.hpp>
//#include <beast/bind/std_placeholders.hpp>
//#include <beast/is_placeholder.hpp>
#include <functional>
#include <type_traits>
#include <utility>

namespace bnet {
namespace beast {
namespace detail {

//------------------------------------------------------------------------------
//
// bind_handler
//
//------------------------------------------------------------------------------

template<class Handler, class... Args>
class bind_wrapper
{
    using args_type = std::tuple<Args...>;

    Handler h_;
    args_type args_;

    template<class T, class Executor>
    friend struct net::associated_executor;

    template<class T, class Allocator>
    friend struct net::associated_allocator;

    template<class T, class CancellationSlot>
    friend struct net::associated_cancellation_slot;

    template<class Arg, class Vals>
    static
    typename std::enable_if<
        std::is_placeholder<typename
            std::decay<Arg>::type>::value == 0 &&
        std::is_placeholder<typename
            std::decay<Arg>::type>::value == 0,
        Arg&&>::type
    extract(Arg&& arg, Vals&& vals)
    {
        bnet::ignore_unused(vals);
        return std::forward<Arg>(arg);
    }

    template<class Arg, class Vals>
    static
    typename std::enable_if<
        std::is_placeholder<typename
            std::decay<Arg>::type>::value != 0,
        std::tuple_element<std::is_placeholder<
            typename std::decay<Arg>::type>::value - 1,
        Vals>>::type&&
    extract(Arg&&, Vals&& vals)
    {
        return std::get<std::is_placeholder<
            typename std::decay<Arg>::type>::value - 1>(
                std::forward<Vals>(vals));
    }

    template<class Arg, class Vals>
    static
    typename std::enable_if<
        std::is_placeholder<typename
            std::decay<Arg>::type>::value != 0 &&
        std::is_placeholder<typename
            std::decay<Arg>::type>::value == 0,
        std::tuple_element<std::is_placeholder<
            typename std::decay<Arg>::type>::value - 1,
        Vals>>::type&&
    extract(Arg&&, Vals&& vals)
    {
        return std::get<std::is_placeholder<
            typename std::decay<Arg>::type>::value - 1>(
                std::forward<Vals>(vals));
    }

    template<class ArgsTuple, std::size_t... S>
    static
    void
    invoke(
        Handler& h,
        ArgsTuple& args,
        std::tuple<>&&,
        mp11::index_sequence<S...>)
    {
        bnet::ignore_unused(args);
        h(std::get<S>(std::move(args))...);
    }

    template<
        class ArgsTuple,
        class ValsTuple,
        std::size_t... S>
    static
    void
    invoke(
        Handler& h,
        ArgsTuple& args,
        ValsTuple&& vals,
        mp11::index_sequence<S...>)
    {
        bnet::ignore_unused(args);
        bnet::ignore_unused(vals);
        h(extract(std::get<S>(std::move(args)),
            std::forward<ValsTuple>(vals))...);
    }

public:
    using result_type = void; // asio needs this

    bind_wrapper(bind_wrapper&&) = default;
    bind_wrapper(bind_wrapper const&) = default;

    template<
        class DeducedHandler,
        class... Args_>
    explicit
    bind_wrapper(
        DeducedHandler&& handler,
        Args_&&... args)
        : h_(std::forward<DeducedHandler>(handler))
        , args_(std::forward<Args_>(args)...)
    {
    }

    template<class... Values>
    void
    operator()(Values&&... values)
    {
        invoke(h_, args_,
            tuple<Values&&...>(
                std::forward<Values>(values)...),
            mp11::index_sequence_for<Args...>());
    }

    //

    template<class Function>
    friend
    net::asio_handler_invoke_is_deprecated
    asio_handler_invoke(
        Function&& f, bind_wrapper* op)
    {
        using net::asio_handler_invoke;
        return asio_handler_invoke(f, std::addressof(op->h_));
    }

    friend
    bool asio_handler_is_continuation(
        bind_wrapper* op)
    {
        using net::asio_handler_is_continuation;
        return asio_handler_is_continuation(
                std::addressof(op->h_));
    }

    friend
    net::asio_handler_allocate_is_deprecated
    asio_handler_allocate(
        std::size_t size, bind_wrapper* op)
    {
        using net::asio_handler_allocate;
        return asio_handler_allocate(
            size, std::addressof(op->h_));
    }

    friend
    net::asio_handler_deallocate_is_deprecated
    asio_handler_deallocate(
        void* p, std::size_t size, bind_wrapper* op)
    {
        using net::asio_handler_deallocate;
        return asio_handler_deallocate(
            p, size, std::addressof(op->h_));
    }
};

template<class Handler, class... Args>
class bind_back_wrapper;

template<class Handler, class... Args>
class bind_front_wrapper;

//------------------------------------------------------------------------------
//
// bind_front
//
//------------------------------------------------------------------------------

template<class Handler, class... Args>
class bind_front_wrapper
{
    Handler h_;
    std::tuple<Args...> args_;

    template<class T, class Executor>
    friend struct net::associated_executor;

    template<class T, class Allocator>
    friend struct net::associated_allocator;

    template<class T, class CancellationSlot>
    friend struct net::associated_cancellation_slot;


    template<std::size_t... I, class... Ts>
    void
    invoke(
        std::false_type,
        mp11::index_sequence<I...>,
        Ts&&... ts)
    {
        h_( std::get<I>(std::move(args_))...,
            std::forward<Ts>(ts)...);
    }

    template<std::size_t... I, class... Ts>
    void
    invoke(
        std::true_type,
        mp11::index_sequence<I...>,
        Ts&&... ts)
    {
        std::mem_fn(h_)(
            std::get<I>(std::move(args_))...,
            std::forward<Ts>(ts)...);
    }

public:
    using result_type = void; // asio needs this

    bind_front_wrapper(bind_front_wrapper&&) = default;
    bind_front_wrapper(bind_front_wrapper const&) = default;

    template<class Handler_, class... Args_>
    bind_front_wrapper(
        Handler_&& handler,
        Args_&&... args)
        : h_(std::forward<Handler_>(handler))
        , args_(std::forward<Args_>(args)...)
    {
    }

    template<class... Ts>
    void operator()(Ts&&... ts)
    {
        invoke(
            std::is_member_function_pointer<Handler>{},
            mp11::index_sequence_for<Args...>{},
            std::forward<Ts>(ts)...);
    }

    //

    template<class Function>
    friend
    net::asio_handler_invoke_is_deprecated
    asio_handler_invoke(
        Function&& f, bind_front_wrapper* op)
    {
        using net::asio_handler_invoke;
        return asio_handler_invoke(f, std::addressof(op->h_));
    }

    friend
    bool asio_handler_is_continuation(
        bind_front_wrapper* op)
    {
        using net::asio_handler_is_continuation;
        return asio_handler_is_continuation(
            std::addressof(op->h_));
    }

    friend
    net::asio_handler_allocate_is_deprecated
    asio_handler_allocate(
        std::size_t size, bind_front_wrapper* op)
    {
        using net::asio_handler_allocate;
        return asio_handler_allocate(
            size, std::addressof(op->h_));
    }

    friend
    net::asio_handler_deallocate_is_deprecated
    asio_handler_deallocate(
        void* p, std::size_t size, bind_front_wrapper* op)
    {
        using net::asio_handler_deallocate;
        return asio_handler_deallocate(
            p, size, std::addressof(op->h_));
    }
};

} // detail
} // beast
} // boost

//------------------------------------------------------------------------------

#ifdef ASIO_STANDALONE
namespace asio {
#else
namespace bnet::asio {
#endif

template<class Handler, class... Args, class Executor>
struct associated_executor<
    bnet::beast::detail::bind_wrapper<Handler, Args...>, Executor>
{
    using type = typename
        associated_executor<Handler, Executor>::type;

    static
    type
    get(bnet::beast::detail::bind_wrapper<Handler, Args...> const& op,
        Executor const& ex = Executor{}) noexcept
    {
        return associated_executor<
            Handler, Executor>::get(op.h_, ex);
    }
};

template<class Handler, class... Args, class Executor>
struct associated_executor<
    bnet::beast::detail::bind_front_wrapper<Handler, Args...>, Executor>
{
    using type = typename
        associated_executor<Handler, Executor>::type;

    static
    type
    get(bnet::beast::detail::bind_front_wrapper<Handler, Args...> const& op,
        Executor const& ex = Executor{}) noexcept
    {
        return associated_executor<
            Handler, Executor>::get(op.h_, ex);
    }
};

//

template<class Handler, class... Args, class Allocator>
struct associated_allocator<
    bnet::beast::detail::bind_wrapper<Handler, Args...>, Allocator>
{
    using type = typename
        associated_allocator<Handler, Allocator>::type;

    static
    type
    get(bnet::beast::detail::bind_wrapper<Handler, Args...> const& op,
        Allocator const& alloc = Allocator{}) noexcept
    {
        return associated_allocator<
            Handler, Allocator>::get(op.h_, alloc);
    }
};

template<class Handler, class... Args, class Allocator>
struct associated_allocator<
    bnet::beast::detail::bind_front_wrapper<Handler, Args...>, Allocator>
{
    using type = typename
        associated_allocator<Handler, Allocator>::type;

    static
    type
    get(bnet::beast::detail::bind_front_wrapper<Handler, Args...> const& op,
        Allocator const& alloc = Allocator{}) noexcept
    {
        return associated_allocator<
            Handler, Allocator>::get(op.h_, alloc);
    }
};

template<class Handler, class... Args, class CancellationSlot>
struct associated_cancellation_slot<
    bnet::beast::detail::bind_wrapper<Handler, Args...>, CancellationSlot>
{
    using type = typename
        associated_cancellation_slot<Handler>::type;

    static
    type
    get(bnet::beast::detail::bind_wrapper<Handler, Args...> const& op,
        CancellationSlot const& slot = CancellationSlot{}) noexcept
    {
        return associated_cancellation_slot<
            Handler, CancellationSlot>::get(op.h_, slot);
    }
};

template<class Handler, class... Args, class CancellationSlot>
struct associated_cancellation_slot<
    bnet::beast::detail::bind_front_wrapper<Handler, Args...>, CancellationSlot>
{
    using type = typename
        associated_cancellation_slot<Handler>::type;

    static
    type
    get(bnet::beast::detail::bind_front_wrapper<Handler, Args...> const& op,
        CancellationSlot const& slot = CancellationSlot{}) noexcept
    {
        return associated_cancellation_slot<
            Handler, CancellationSlot>::get(op.h_, slot);
    }
};



#ifdef ASIO_STANDALONE
}
#else
}}
#endif

//------------------------------------------------------------------------------

namespace std {

// VFALCO Using std::bind on a completion handler will
// cause undefined behavior later, because the executor
// associated with the handler is not propagated to the
// wrapper returned by std::bind; these overloads are
// deleted to prevent mistakes. If this creates a problem
// please contact me.

template<class Handler, class... Args>
void
bind(bnet::beast::detail::bind_wrapper<
    Handler, Args...>, ...) = delete;

template<class Handler, class... Args>
void
bind(bnet::beast::detail::bind_front_wrapper<
    Handler, Args...>, ...) = delete;

} // std

//------------------------------------------------------------------------------

#endif
