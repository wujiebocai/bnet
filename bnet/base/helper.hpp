#pragma once

#include "base/traits.hpp"

namespace bnet::base {
    template <class StreamType>
    concept is_binary_stream = requires {
        typename StreamType::stream_type;
        requires std::is_same_v<unqualified_t<typename StreamType::stream_type>, void>;
    };

#if defined(NET_USE_SSL)
    template <class StreamType>
    concept is_ssl_stream = requires {
        typename StreamType::stream_type;
        requires std::is_same_v<unqualified_t<typename StreamType::stream_type>, asio::ssl::stream<asio::ip::tcp::socket>>;
    };
#endif

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
}