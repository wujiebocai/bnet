#pragma once
#include <iostream>
#include "bnet.hpp"
#include "other.hpp"
using namespace bnet;

#if defined(BNET_ENABLE_SSL)

using s_session_ptr_type = typename tcps_svr::session_ptr_type;
using c_session_ptr_type = typename tcps_cli::session_ptr_type;

inline void s_connect_cb([[maybe_unused]]s_session_ptr_type& ptr) {
	std::cout << "svr connect success" << std::endl;
}

inline void s_disconnect_cb([[maybe_unused]]s_session_ptr_type& ptr, error_code ec) {
	std::cout << "svr disconnect" << ec.message() << std::endl;
}

inline void s_handshake_cb([[maybe_unused]]s_session_ptr_type& ptr, error_code ec) {
	std::cout << "svr handshake" << ec.message() << std::endl;
}

inline void s_recv_cb([[maybe_unused]]s_session_ptr_type& ptr, std::string_view&& s) {
	ptr->send(std::move(s));
}
//////////////////////////////////////////////////////////////////////////////////
inline void c_connect_cb([[maybe_unused]]c_session_ptr_type& ptr) {
	std::cout << "client connect success" << std::endl;
	ptr->send("a");
}

inline void c_disconnect_cb([[maybe_unused]]c_session_ptr_type& ptr, error_code ec) {
	std::cout << "client disconnect" << ec.message() << std::endl;
}

inline void c_handshake_cb([[maybe_unused]]c_session_ptr_type& ptr, error_code ec) {
	std::cout << "client handshake" << ec.message() << std::endl;
}

inline void c_recv_cb([[maybe_unused]]c_session_ptr_type& ptr, std::string_view&& s) {
	ptr->send(std::move(s));
}

void tcps_tst(asio::io_context& io) {
	svr_cfg scfg {
		thread_num: 8,
	};
	cli_cfg ccfg {
		thread_num: 5,
		pool_size: 100
	};
	// svr
  	static auto tcpssvr = tcps_svr(scfg);
    tcpssvr.bind(event::connect, s_connect_cb);
    tcpssvr.bind(event::disconnect, s_disconnect_cb);
    tcpssvr.bind(event::handshake, s_handshake_cb);
    tcpssvr.bind(event::recv, s_recv_cb);
    tcpssvr.set_cert_buffer(ca_crt, server_crt, server_key, "123456");
    tcpssvr.set_dh_buffer(dh);
  	tcpssvr.start();

	//cli
	static auto tcpscli = tcps_cli(ccfg);
    tcpscli.bind(event::connect, c_connect_cb);
    tcpscli.bind(event::disconnect, c_disconnect_cb);
    tcpscli.bind(event::handshake, c_handshake_cb);
    tcpscli.bind(event::recv, c_recv_cb);
    tcpscli.set_verify_mode(asio::ssl::verify_peer);
    tcpscli.set_cert_buffer(ca_crt, client_crt, client_key, "123456");
	tcpscli.start();
}

#endif