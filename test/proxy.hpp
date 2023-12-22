#pragma once

#include "bnet.hpp"
#include "other.hpp"

using namespace bnet;

template<class SvrType>
class svr_proxy : public SvrType {
public:
	using session_ptr_type = typename SvrType::session_ptr_type;
public:
	svr_proxy(const svr_cfg& cfg) 
		: SvrType(cfg) {

		this->bind(event::connect, [&]([[maybe_unused]]session_ptr_type& ptr) {
			std::cout << "connect success" << ", " << this->session_count() << std::endl;
		});

		// only ssl or kcp call back
		this->bind(event::handshake, []([[maybe_unused]]session_ptr_type& ptr, error_code ec) { 
			std::cout << "handshake " << ec.message() << std::endl;
		});
		// only ws call back
		this->bind(event::upgrade, &svr_proxy::upgrade, this); 
		
		this->bind(event::disconnect, []([[maybe_unused]]session_ptr_type& ptr, error_code ec) {
			std::cout << "disconnect " << ec.message() << std::endl;
		});
		this->bind(event::recv, [&]([[maybe_unused]]session_ptr_type& ptr, std::string_view&& s) {
			ptr->send(std::move(s));
		});
	}

	inline void upgrade([[maybe_unused]]session_ptr_type& ptr, error_code ec) {
		std::cout << "upgrade " << ec.message() << std::endl;
	}
};

template<class CliType>
class cli_proxy : public CliType {
public:
	using session_ptr_type = typename CliType::session_ptr_type;
public:
	cli_proxy(const cli_cfg& cfg)
		: CliType(cfg) {

		this->bind(event::connect, []([[maybe_unused]]session_ptr_type& ptr) {
			std::cout << "client connect success" << std::endl;

			ptr->send("a");
		});

		// only ssl or kcp call back
		this->bind(event::handshake, []([[maybe_unused]]session_ptr_type& ptr, error_code ec) { 
			std::cout << "client handshake " << ec.message() << std::endl;
		});
		// only ws call back
		this->bind(event::upgrade, []([[maybe_unused]]session_ptr_type& ptr, error_code ec) { 
			std::cout << "client upgrade " << ec.message() << std::endl;
		});

		this->bind(event::disconnect, []([[maybe_unused]]session_ptr_type& ptr, error_code ec) {
			std::cout << "client disconnect " << ec.message() << std::endl;
			if (ec) { // reconnection
				ptr->reconn();
			}
		});

		this->bind(event::recv, []([[maybe_unused]]session_ptr_type& ptr, std::string_view&& s) {
			ptr->send(std::move(s));
		});
	}
};

template<class SvrType, class CliType, bool IsSsl = false>
void proxy_tst(asio::io_context& io) {
	svr_cfg scfg {
		thread_num: 8,
	};
	cli_cfg ccfg {
		thread_num: 5,
		pool_size: 100
	};
	// svr
	static auto svr = svr_proxy<SvrType>(scfg);
	if constexpr (IsSsl) {
		svr.set_cert_buffer(ca_crt, server_crt, server_key, "123456");
    	svr.set_dh_buffer(dh);
	}
	svr.start();

    // cli
    static auto cli = cli_proxy<CliType>(ccfg);
	if constexpr (IsSsl) {
		cli.set_verify_mode(asio::ssl::verify_peer);
    	cli.set_cert_buffer(ca_crt, client_crt, client_key, "123456");
	}
    cli.start();

	/*
	* The following code has the same effect as setting the pool_size configuration field, except that different service ports can be connected
	*/
	//for (int i = 0; i < 10; ++i) {
	//	cli.add("127.0.0.1", "10240");
	//}
}

/*
* the following parameters correspond to different test cases:
* tcp, false, io -> test tcp
* tcps, true, io -> test tcp ssl
* ws, false, io -> test websocket
* wss, true, io -> test websocket ssl
* udp, false, io -> test udp
* kcp, false, io -> test kcp
*/
#define net_proxy_tst(proto, is_ssl, io) proxy_tst<proto##_svr, proto##_cli, is_ssl>(io)
