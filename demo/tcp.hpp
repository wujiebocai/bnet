#pragma once

#include <iostream>
#include "bnet.hpp"
using namespace bnet;

struct cb_event {
	using s_session_ptr_type = typename tcp_svr::session_ptr_type;
	using c_session_ptr_type = typename tcp_cli::session_ptr_type;

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
		std::string msgdata(1024, 'a');
		ptr->send("a");
	}

    inline static void c_disconnect_cb([[maybe_unused]]c_session_ptr_type& ptr, error_code ec) {
		std::cout << "client disconnect" << ec.message() << std::endl;
	}

    inline static void c_recv_cb([[maybe_unused]]c_session_ptr_type& ptr, std::string_view&& s) {
		ptr->send(std::move(s));
	}
};

void tcp_tst(asio::io_context& io) {
	svr_cfg scfg {
		thread_num: 8,
	};
	cli_cfg ccfg {
		thread_num: 5,
		pool_size: 100,
		is_reconn: true
	};
    // svr
  	static auto tcpsvr = tcp_svr(scfg);
    tcpsvr.bind(event::connect, &cb_event::s_connect_cb); // you can also use closures directly or member function
    tcpsvr.bind(event::disconnect, &cb_event::s_disconnect_cb);
    tcpsvr.bind(event::recv, &cb_event::s_recv_cb);
  	tcpsvr.start();

	//cli
	static auto tcpcli = tcp_cli(ccfg);
    tcpcli.bind(event::connect, &cb_event::c_connect_cb);
    tcpcli.bind(event::disconnect, &cb_event::c_disconnect_cb);
    tcpcli.bind(event::recv, &cb_event::c_recv_cb);
	tcpcli.start();

	/*
	* The following code has the same effect as setting the pool_size configuration field, except that different service ports can be connected
	*/
	for (int i = 0; i < 10; ++i) {
		tcpcli.add("127.0.0.1", "10240");
	}
}