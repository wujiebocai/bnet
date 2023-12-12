#pragma once
#include <iostream>
#include "bnet.hpp"
using namespace bnet;

struct cb_event {
    using s_session_ptr_type = typename udp_svr::session_ptr_type;
    using c_session_ptr_type = typename udp_cli::session_ptr_type;

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
        std::cout << "client recv " << s << std::endl;
		ptr->send(std::move(s));
	}
};

void udp_tst() {
    svr_cfg scfg {
		thread_num: 8,
	};
	cli_cfg ccfg {
		thread_num: 5,
		pool_size: 100
	};

    cb_event ev;
	// svr
  	static auto udpsvr = udp_svr(scfg);
    udpsvr.bind(event::connect, &cb_event::s_connect_cb, &ev);
    udpsvr.bind(event::disconnect, &cb_event::s_disconnect_cb, &ev);
    udpsvr.bind(event::handshake, &cb_event::s_handshake_cb, &ev);
    udpsvr.bind(event::recv, &cb_event::s_recv_cb, &ev);
  	udpsvr.start();

	//cli
	static auto udpcli = udp_cli(ccfg);
    udpcli.bind(event::connect, &cb_event::c_connect_cb, &ev);
    udpcli.bind(event::disconnect, &cb_event::c_disconnect_cb, &ev);
    udpcli.bind(event::handshake, &cb_event::c_handshake_cb, &ev);
    udpcli.bind(event::recv, &cb_event::c_recv_cb, &ev);
	udpcli.start();

}