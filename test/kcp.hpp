#pragma once
#include <iostream>
#include "bnet.hpp"
using namespace bnet;

struct cb_event {
	using s_session_ptr_type = typename kcp_svr::session_ptr_type;
	using c_session_ptr_type = typename kcp_cli::session_ptr_type;

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
		//ptr->send("a");
		//ptr->stop(asio::error::operation_aborted);
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
};

void kcp_tst() {
	svr_cfg scfg {
		thread_num: 8,
	};
	cli_cfg ccfg {
		thread_num: 5,
		pool_size: 10000,
		is_reconn: true
	};

    cb_event ev;
	// svr
  	static auto kcpsvr = kcp_svr(scfg);
    kcpsvr.bind(event::connect, &cb_event::s_connect_cb, &ev);
    kcpsvr.bind(event::disconnect, &cb_event::s_disconnect_cb, &ev);
    kcpsvr.bind(event::handshake, &cb_event::s_handshake_cb, &ev);
    kcpsvr.bind(event::recv, &cb_event::s_recv_cb, &ev);
  	kcpsvr.start();

	//cli
	static auto kcpcli = kcp_cli(ccfg);
    kcpcli.bind(event::connect, &cb_event::c_connect_cb, &ev);
    kcpcli.bind(event::disconnect, &cb_event::c_disconnect_cb, &ev);
    kcpcli.bind(event::handshake, &cb_event::c_handshake_cb, &ev);
    kcpcli.bind(event::recv, &cb_event::c_recv_cb, &ev);
	kcpcli.start();
}