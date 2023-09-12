#pragma once
#include "bnet.hpp"
#include "other.hpp"
using namespace bnet;

template<class SvrType>
class svr_proxy : public SvrType {
public:
	using session_ptr_type = typename SvrType::session_ptr_type;
public:
	svr_proxy(std::size_t concurrency = std::thread::hardware_concurrency() * 2, std::size_t max_buffer_size = 65535) 
		: SvrType(concurrency, max_buffer_size) {

		this->bind(event::connect, [&]([[maybe_unused]]session_ptr_type& ptr) {
			std::cout << "connect success" << ", " << this->session_count() << std::endl;
			//ptr->stop(ec);
		});
		this->bind(event::handshake, []([[maybe_unused]]session_ptr_type& ptr, error_code ec) { // only ssl or kcp call back
			std::cout << "handshake" << ec.message() << std::endl;
		});
		this->bind(event::upgrade, &svr_proxy::upgrade, this); // only ws call back
		this->bind(event::disconnect, []([[maybe_unused]]session_ptr_type& ptr, error_code ec) {
			std::cout << "disconnect" << ec.message() << std::endl;
		});
		this->bind(event::recv, [&]([[maybe_unused]]session_ptr_type& ptr, std::string_view&& s) {
			ptr->send(std::move(s));
		});
	}

	inline void upgrade([[maybe_unused]]session_ptr_type& ptr, error_code ec) {
		std::cout << "upgrade" << ec.message() << std::endl;
	}
};

template<class CliType>
class cli_proxy : public CliType {
public:
	using session_ptr_type = typename CliType::session_ptr_type;
public:
	cli_proxy(std::size_t concurrency = std::thread::hardware_concurrency() * 2, std::size_t max_buffer_size = 65535)
		: CliType(concurrency, max_buffer_size) {

		this->bind(event::connect, []([[maybe_unused]]session_ptr_type& ptr) {
			std::cout << "client connect success" << std::endl;

			ptr->send("a");
		});
		this->bind(event::handshake, []([[maybe_unused]]session_ptr_type& ptr, error_code ec) { // only ssl or kcp call back
			std::cout << "client handshake" << ec.message() << std::endl;
		});
		this->bind(event::upgrade, ([[maybe_unused]]session_ptr_type& ptr, error_code ec) { // only ws call back
			std::cout << "client upgrade" << ec.message() << std::endl;
		});
		this->bind(event::disconnect, []([[maybe_unused]]session_ptr_type& ptr, error_code ec) {
			std::cout << "client disconnect" << ec.message() << std::endl;
			if (ec) { // reconnection
				ptr->reconn();
			}
		});
		this->bind(event::recv, []([[maybe_unused]]session_ptr_type& ptr, std::string_view&& s) {
			ptr->send(std::move(s));
		});
	}
};

// proxy test
void proxy_tst() {
	// tcp svr
	static auto tcpsvr = svr_proxy<tcp_svr>(2);
	tcpsvr.start("0.0.0.0", "8880");

    // tcp cli
    static auto tcpcli = cli_proxy<tcp_cli>(2);
    tcpcli.start();
    for (int i = 0; i < 50; ++i) {
        tcpcli.add("192.168.152.62", "8880");
    }
///////////////////////////////////////////////////////////
#if defined(BNET_ENABLE_SSL)
	// tcps svr
  	static auto tcpssvr = svr_proxy<tcps_svr>(2);
    tcpssvr.set_cert_buffer(ca_crt, server_crt, server_key, "123456");
    tcpssvr.set_dh_buffer(dh);
  	tcpssvr.start("0.0.0.0", "8881");

	// tcps cli
	static auto tcpscli = cli_proxy<tcps_cli>(2);
    tcpscli.set_verify_mode(asio::ssl::verify_peer);
    tcpscli.set_cert_buffer(ca_crt, client_crt, client_key, "123456");
	tcpscli.start();
	for (int i = 0; i < 50; ++i) {
		tcpscli.add("127.0.0.1", "8881");
	}
#endif
/////////////////////////////////////////////////////////////
	// udp svr
	static auto udpsvr = svr_proxy<udp_svr>(2);
	udpsvr.start("0.0.0.0", "8882");
	
    // udp cli
    static auto udpcli = cli_proxy<udp_cli>(2);
    udpcli.start();
    for (int i = 0; i < 50; ++i) {
        udpcli.add("127.0.0.1", "8882");
    }
///////////////////////////////////////////////////////////////
	// kcp svr
  	static auto kcpsvr = svr_proxy<kcp_svr>(2);
  	kcpsvr.start("0.0.0.0", "8883");

	// kcp cli
	static auto kcpcli = cli_proxy<kcp_cli>(2);
	kcpcli.start();
	for (int i = 0; i < 5000; ++i) {
		kcpcli.add("127.0.0.1", "8883");
	}
///////////////////////////////////////////////////////////////
#if defined(BNET_ENABLE_HTTP)
	// ws svr
  	static auto wssvr = svr_proxy<ws_svr>(2);
  	wssvr.start("0.0.0.0", "8884");

	// ws cli
	static auto wscli = cli_proxy<ws_cli>(2);
	wscli.start();
	for (int i = 0; i < 5000; ++i) {
		wscli.add("127.0.0.1", "8884");
	}
#endif
/////////////////////////////////////////////////////////////////
#if defined(BNET_ENABLE_SSL) && defined(BNET_ENABLE_HTTP)
	// wss svr
  	static auto wsssvr = svr_proxy<wss_svr>(2);
    wsssvr.set_cert_buffer(ca_crt, server_crt, server_key, "123456");
    wsssvr.set_dh_buffer(dh);
  	wsssvr.start("0.0.0.0", "8885");

	// wss cli
	static auto wsscli = cli_proxy<wss_cli>(2);
    wsscli.set_verify_mode(asio::ssl::verify_peer);
    wsscli.set_cert_buffer(ca_crt, client_crt, client_key, "123456");
	wsscli.start();
	for (int i = 0; i < 50; ++i) {
		wsscli.add("127.0.0.1", "8885");
	}
#endif
}
