#pragma once
#include <iostream>
#include "bnet.hpp"
#include "other.hpp"
using namespace bnet;

#if defined(BNET_ENABLE_SSL) && defined(BNET_ENABLE_HTTP)
using s_session_ptr_type = typename wss_svr::session_ptr_type;
using c_session_ptr_type = typename wss_cli::session_ptr_type;
struct cb_event {
    inline static void s_connect_cb([[maybe_unused]]s_session_ptr_type& ptr) {
		std::cout << "svr connect success" << std::endl;
	}

    inline static void s_disconnect_cb([[maybe_unused]]s_session_ptr_type& ptr, error_code ec) {
		std::cout << "svr disconnect" << ec.message() << std::endl;
	}

    inline static void s_handshake_cb([[maybe_unused]]s_session_ptr_type& ptr, error_code ec) {
    	std::cout << "svr handshake" << ec.message() << std::endl;
    }

    inline static void s_upgrade_cb([[maybe_unused]]s_session_ptr_type& ptr, error_code ec) {
		std::cout << "svr upgrade" << ec.message() << std::endl;
	}

    inline static void s_recv_cb([[maybe_unused]]s_session_ptr_type& ptr, std::string_view&& s) {
		ptr->send(std::move(s));
	}
//////////////////////////////////////////////////////////////////////////////////
    inline static void c_connect_cb([[maybe_unused]]c_session_ptr_type& ptr) {
		std::cout << "client connect success" << std::endl;
	}

    inline static void c_disconnect_cb([[maybe_unused]]c_session_ptr_type& ptr, error_code ec) {
		std::cout << "client disconnect" << ec.message() << std::endl;
	}

    inline static void c_handshake_cb([[maybe_unused]]c_session_ptr_type& ptr, error_code ec) {
    	std::cout << "client handshake" << ec.message() << std::endl;
    }

    inline static void c_upgrade_cb([[maybe_unused]]c_session_ptr_type& ptr, error_code ec) {
		std::cout << "client upgrade" << ec.message() << std::endl;
	}

    inline static void c_recv_cb([[maybe_unused]]c_session_ptr_type& ptr, std::string_view&& s) {
		ptr->send(std::move(s));
	}
};

void wss_tst() {
	// svr
  	static auto wsssvr = wss_svr(8);
    wsssvr.bind(event::connect, &cb_event::s_connect_cb);
    wsssvr.bind(event::disconnect, &cb_event::s_disconnect_cb);
    wsssvr.bind(event::handshake, &cb_event::s_handshake_cb);
    wsssvr.bind(event::upgrade, &cb_event::s_upgrade_cb);
    wsssvr.bind(event::recv, &cb_event::s_recv_cb);
    wsssvr.set_cert_buffer(ca_crt, server_crt, server_key, "123456");
    wsssvr.set_dh_buffer(dh);
  	wsssvr.start("0.0.0.0", "8888");

	//cli
	static auto wsscli = wss_cli(5);
    wsscli.bind(event::connect, &cb_event::c_connect_cb);
    wsscli.bind(event::disconnect, &cb_event::c_disconnect_cb);
    wsscli.bind(event::handshake, &cb_event::c_handshake_cb);
    wsscli.bind(event::upgrade, &cb_event::c_upgrade_cb);
    wsscli.bind(event::recv, &cb_event::c_recv_cb);
    wsscli.set_verify_mode(asio::ssl::verify_peer);
    wsscli.set_cert_buffer(ca_crt, client_crt, client_key, "123456");
	wsscli.start();
    // cli connect
	for (int i = 0; i < 5000; ++i) {
		wsscli.add("127.0.0.1", "8888");
	}
}
#endif