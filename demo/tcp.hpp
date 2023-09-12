#pragma once
#include <iostream>
#include "bnet.hpp"
using namespace bnet;

using s_session_ptr_type = typename tcp_svr::session_ptr_type;
using c_session_ptr_type = typename tcp_cli::session_ptr_type;

struct cb_event {
    inline static void s_connect_cb([[maybe_unused]]s_session_ptr_type& ptr) {
		std::cout << "svr connect success" << std::endl;
	}

    inline static void s_disconnect_cb([[maybe_unused]]s_session_ptr_type& ptr, error_code ec) {
		std::cout << "svr disconnect" << ec.message() << std::endl;
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

    inline static void c_recv_cb([[maybe_unused]]c_session_ptr_type& ptr, std::string_view&& s) {
		ptr->send(std::move(s));
	}
};

void tcp_tst() {
    // svr
  	static auto tcpsvr = tcp_svr(8);
    tcpsvr.bind(event::connect, &cb_event::s_connect_cb); // You can also use closures directly
    tcpsvr.bind(event::disconnect, &cb_event::s_disconnect_cb);
    tcpsvr.bind(event::recv, &cb_event::s_recv_cb);
  	tcpsvr.start("0.0.0.0", "8888");

	//cli
	static auto tcpcli = tcp_cli(5);
    tcpcli.bind(event::connect, &cb_event::c_connect_cb);
    tcpcli.bind(event::disconnect, &cb_event::c_disconnect_cb);
    tcpcli.bind(event::recv, &cb_event::c_recv_cb);
	tcpcli.start();
    // cli connect
	for (int i = 0; i < 5000; ++i) {
		tcpcli.add("127.0.0.1", "8888");
	}
}