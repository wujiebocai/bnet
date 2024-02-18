/*
 * Copyright (c) 2023 bocai
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include "base/traits.hpp"

namespace bnet::base {
    template <class StreamType>
    concept is_binary_stream = requires {
        typename StreamType::stream_type;
        requires std::is_same_v<unqualified_t<typename StreamType::stream_type>, void>;
    };

#if defined(BNET_ENABLE_SSL)
    template <class StreamType>
    concept is_ssl_stream = requires {
        typename StreamType::stream_type;
        requires std::is_same_v<unqualified_t<typename StreamType::stream_type>, asio::ssl::stream<asio::ip::tcp::socket>>;
    };
#endif

    struct svr_tag {};
	struct cli_tag {};

	struct kcp_stream_tag {};
	struct http_tag {};
    
	struct ws_tag {};

    template <class StreamType>
    concept is_kcp_stream = requires {
        typename StreamType::stream_type;
        requires std::is_same_v<unqualified_t<typename StreamType::stream_type>, kcp_stream_tag>;
    };

    template <class StreamType>
    concept is_tcp_by_stream = requires {
        typename StreamType::socket_type;
        requires std::is_same_v<typename unqualified_t<typename StreamType::socket_type>::protocol_type, asio::ip::tcp>;
    };
    //concept is_tcp = std::is_same_v<typename unqualified_t<typename StreamType::socket_type>::protocol_type, asio::ip::tcp>;

    template <class StreamType>
    concept is_udp_by_stream = requires {
        typename StreamType::socket_type;
        requires std::is_same_v<typename unqualified_t<typename StreamType::socket_type>::protocol_type, asio::ip::udp>;
    };

    template<class SocketType>
	constexpr bool is_tcp_by_socket = std::is_same_v<typename unqualified_t<SocketType>::protocol_type, asio::ip::tcp>;
	template<class SocketType>
	constexpr bool is_udp_by_socket = std::is_same_v<typename unqualified_t<SocketType>::protocol_type, asio::ip::udp>;

    template <typename Proto>
    concept is_http_proto = std::is_base_of_v<http_tag, Proto>;
    template <typename Proto>
    concept is_ws_proto = std::is_base_of_v<ws_tag, Proto>;

    template <typename T, typename Ptr>
    concept proto_able  = requires(T t, Ptr ptr) {
        { t.template proto_start(ptr) } -> std::same_as<asio::awaitable<error_code>>;
        { t.template proto_stop(ptr) } -> std::same_as<asio::awaitable<error_code>>;
    };

    template <typename... Ts>
	inline constexpr void ignore_unused(Ts const& ...) noexcept {}

	template <typename... Ts>
	inline constexpr void ignore_unused() noexcept {}

    template<class...> inline constexpr bool always_false_v = false;

    template <typename Enumeration>
	inline constexpr auto to_underlying(Enumeration const value) noexcept ->
		typename std::underlying_type<Enumeration>::type {
		return static_cast<typename std::underlying_type<Enumeration>::type>(value);
	}

    template<class SvrOrCli>
	concept is_svr_c = std::is_same_v<SvrOrCli, svr_tag>;
	template<class SvrOrCli>
	concept is_cli_c = std::is_same_v<SvrOrCli, cli_tag>;

    template<typename T>
	concept is_co_spawn_cb = requires(T a, std::exception_ptr ex) {
    	{ a(ex, error_code()) } -> std::same_as<void>;
	};

    template<typename Proto>
    concept is_rpc_proto = requires(Proto p) { 
        typename Proto::req_header;
        typename Proto::rsp_header;
        requires requires(typename Proto::req_header req, typename Proto::rsp_header rsp) {
            { req.length } -> std::convertible_to<uint32_t>;
            { rsp.length } -> std::convertible_to<uint32_t>;
        };
    };

    template<typename Proto>
    concept is_txt_proto_match = requires(typename Proto::result_type ret) { 
        { Proto::match_func(asio::buffers_iterator<asio::streambuf::const_buffers_type>{}, asio::buffers_iterator<asio::streambuf::const_buffers_type>{}, ret) } -> 
            std::same_as<std::pair<asio::buffers_iterator<asio::streambuf::const_buffers_type>, bool>>;
    };

    template<typename Proto>
    concept is_txt_proto_cli_delim = requires { 
        { Proto::cli_delim() } -> std::convertible_to<std::string>;
    };

    template<typename Proto>
    concept is_txt_proto_svr_delim = requires { 
        { Proto::svr_delim() } -> std::convertible_to<std::string>;
    };

    template<typename Proto>
    concept is_txt_proto = is_txt_proto_match<Proto> || is_txt_proto_cli_delim<Proto>;
}