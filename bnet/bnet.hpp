#pragma once

#include <asio.hpp>
#if defined(NET_USE_SSL)
#include <asio/ssl.hpp>
#endif

#if defined(NET_USE_HTTP)
#ifndef BEAST_HEADER_ONLY
#define BEAST_HEADER_ONLY 1
#endif
#include <beast/beast.hpp>
#if defined(NET_USE_SSL)
#if defined(BEAST_VERSION) && (BEAST_VERSION >= 277)
	#include <beast/beast/ssl.hpp>
#endif
	#include <beast/beast/websocket/ssl.hpp>
#endif
namespace bnet {
	namespace http = bnet::beast::http;
	namespace websocket = bnet::beast::websocket;
}
#endif

#include "base/timer.hpp"
#include "base/server.hpp"
#include "base/client.hpp"
#include "base/httpserver.hpp"
#include "base/httpclient.hpp"

/*
net：
socket type: tcp, udp
stream type: binary, ssl, kcp, ...
protocol type: http, websocket, ...
*/

namespace bnet {
// socket type
using tcp_socket = base::bsocket<asio::ip::tcp::socket>;
using udp_socket_s = base::bsocket<asio::ip::udp::socket&>;
using udp_socket_c = base::bsocket<asio::ip::udp::socket>;
#if defined(NET_USE_SSL)
using ssl_socket = base::bsocket<asio::ssl::stream<asio::ip::tcp::socket>>;
#endif
#if defined(NET_USE_HTTP)
using ws_socket = base::bws_socket<beast::websocket::stream<asio::ip::tcp::socket&>>;
#if defined(NET_USE_SSL)
using wss_socket = base::bws_socket<beast::websocket::stream<asio::ssl::stream<asio::ip::tcp::socket>&>>;
#endif
#endif

// stream type
using tcp_binary_stream = base::binary_stream<tcp_socket>;
using udp_binary_stream_s = base::binary_stream<udp_socket_s>;
using udp_binary_stream_c = base::binary_stream<udp_socket_c>;
#if defined(NET_USE_SSL)
using tcp_ssl_stream = base::ssl_stream<ssl_socket>;
#endif
#if defined(NET_USE_HTTP)
using ws_stream = base::binary_stream<ws_socket>;
#if defined(NET_USE_SSL)
using wss_stream = base::ssl_stream<wss_socket>;
#endif
#endif
using kcp_stream_s = base::kcp_stream<udp_socket_s, base::svr_tag>;
using kcp_stream_c = base::kcp_stream<udp_socket_c, base::cli_tag>;

// proto type
using default_proto = base::proto<>;
#if defined(NET_USE_HTTP)
using http_proto_s = base::http_proto<base::svr_tag>;
using http_proto_c = base::http_proto<base::cli_tag>;
using ws_proto_s = base::ws_proto<base::svr_tag>;
using ws_proto_c = base::ws_proto<base::cli_tag>;
#endif

using base::Timer;

// tcp
using tcp_svr = base::server<tcp_binary_stream, default_proto>;
using tcp_cli = base::client<tcp_binary_stream, default_proto>;

// tcps
#if defined(NET_USE_SSL)
using tcps_svr = base::server<tcp_ssl_stream, default_proto>;
using tcps_cli = base::client<tcp_ssl_stream, default_proto>;
#endif

// udp
using udp_svr = base::server<udp_binary_stream_s, default_proto>;
using udp_cli = base::client<udp_binary_stream_c, default_proto>;

//kcp
using kcp_svr = base::server<kcp_stream_s, default_proto>;
using kcp_cli = base::client<kcp_stream_c, default_proto>;

// ws
#if defined(NET_USE_HTTP)
using ws_svr = base::server<ws_stream, ws_proto_s>;
using ws_cli = base::client<ws_stream, ws_proto_c>;
#endif

// wss
#if defined(NET_USE_SSL) && defined(NET_USE_HTTP)
using wss_svr = base::server<wss_stream, ws_proto_s>;
using wss_cli = base::client<wss_stream, ws_proto_c>;
#endif

// http
#if defined(NET_USE_HTTP)
using http_svr = base::http_server<tcp_binary_stream, http_proto_s>;
using http_cli = base::http_client<tcp_binary_stream, http_proto_c>;
#endif

// https
#if defined(NET_USE_SSL) && defined(NET_USE_HTTP)
using https_svr = base::http_server<tcp_ssl_stream, http_proto_s>;
using https_cli = base::http_client<tcp_ssl_stream, http_proto_c>;
#endif

}
