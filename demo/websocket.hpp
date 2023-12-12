#pragma once
#include <iostream>
#include "bnet.hpp"
using namespace bnet;

#if defined(BNET_ENABLE_HTTP)
struct cb_event {
	using s_session_ptr_type = typename ws_svr::session_ptr_type;
	using c_session_ptr_type = typename ws_cli::session_ptr_type;

    inline void s_connect_cb([[maybe_unused]]s_session_ptr_type& ptr) {
		std::cout << "svr connect success" << std::endl;
	}

    inline void s_disconnect_cb([[maybe_unused]]s_session_ptr_type& ptr, error_code ec) {
		std::cout << "svr disconnect" << ec.message() << std::endl;
	}

    inline void s_upgrade_cb([[maybe_unused]]s_session_ptr_type& ptr, error_code ec) {
		std::cout << "svr upgrade" << ec.message() << std::endl;
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

    inline void c_upgrade_cb([[maybe_unused]]c_session_ptr_type& ptr, error_code ec) {
		std::cout << "client upgrade" << ec.message() << std::endl;
	}

    inline void c_recv_cb([[maybe_unused]]c_session_ptr_type& ptr, std::string_view&& s) {
		ptr->send(std::move(s));
	}
};

void ws_tst(asio::io_context& io) {
	svr_cfg scfg {
		thread_num: 8,
	};
	cli_cfg ccfg {
		thread_num: 5,
		pool_size: 100
	};
    cb_event ev;
	// svr
  	static auto wssvr = ws_svr(scfg);
    wssvr.bind(event::connect, &cb_event::s_connect_cb, &ev);
    wssvr.bind(event::disconnect, &cb_event::s_disconnect_cb, &ev);
    wssvr.bind(event::upgrade, &cb_event::s_upgrade_cb, &ev);
    wssvr.bind(event::recv, &cb_event::s_recv_cb, &ev);
  	wssvr.start();

	//cli
	static auto wscli = ws_cli(ccfg);
    wscli.bind(event::connect, &cb_event::c_connect_cb, &ev);
    wscli.bind(event::disconnect, &cb_event::c_disconnect_cb, &ev);
    wscli.bind(event::upgrade, &cb_event::c_upgrade_cb, &ev);
    wscli.bind(event::recv, &cb_event::c_recv_cb, &ev);
	wscli.start();
}

#endif