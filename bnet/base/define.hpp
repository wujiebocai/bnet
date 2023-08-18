#pragma once

namespace bnet::base {
	enum class estate : std::int8_t { stopped, stopping, starting, started };
	// 回调事件
	enum class event : std::int8_t {
		init,
		connect,
		disconnect,
		recv,
		packet,
		handshake,
		upgrade,
		max
	};

	static long constexpr http_execute_timeout   = 5 * 1000;

	using CBPROXYTYPE = func_proxy_imp<event>;
	typedef std::shared_ptr<CBPROXYTYPE> func_proxy_imp_ptr;

	struct svr_tag {};
	struct cli_tag {};
	struct kcp_stream_tag {};

	struct http_tag {};
	struct ws_tag {};

	//template<class PROTOCOLTYPE>
	//constexpr bool is_http_protocoltype_v = std::is_same_v<PROTOCOLTYPE, http_proto_flag>;
	//template<class PROTOCOLTYPE>
	//constexpr bool is_websocket_protocoltype_v = std::is_same_v<PROTOCOLTYPE, websocket_proto_flag>;

	//template<class STREAMTYPE>
	//constexpr bool is_binary_streamtype_v = std::is_same_v<STREAMTYPE, binary_stream_flag>;
#if defined(NET_USE_SSL)
	//template<class STREAMTYPE>
	//constexpr bool is_ssl_streamtype_v = std::is_same_v<STREAMTYPE, asio::ssl::stream<asio::ip::tcp::socket&>>;
#endif
	//template<class STREAMTYPE>
	//constexpr bool is_kcp_streamtype_v = std::is_same_v<STREAMTYPE, kcp_stream_flag>;
#if defined(NET_USE_HTTP)
	//template<class STREAMTYPE>
	//constexpr bool is_ws_streamtype_v = std::is_same_v<STREAMTYPE, websocket::stream<asio::ip::tcp::socket&>>;
#else 
	//template<class STREAMTYPE>
	//constexpr bool is_ws_streamtype_v = false;
#endif
#if defined(NET_USE_SSL) && defined(NET_USE_HTTP)
	//template<class STREAMTYPE>
	//constexpr bool is_wss_streamtype_v = std::is_same_v<STREAMTYPE, websocket::stream<asio::ssl::stream<asio::ip::tcp::socket&>&>>;
#else
	//template<class STREAMTYPE>
	//constexpr bool is_wss_streamtype_v = false;
#endif
}

