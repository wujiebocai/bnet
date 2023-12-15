#pragma once
#include <iostream>
#include "bnet.hpp"
using namespace bnet;

#if defined(BNET_ENABLE_HTTP)
void http_tst() {
	svr_cfg scfg {
		thread_num: 8,
	};
	cli_cfg ccfg {
		thread_num: 5,
		pool_size: 100,
		is_reconn: true,
		is_async: false
	};
    // svr
  	static auto httpsvr = http_svr(scfg);
  	httpsvr.start();
    // route
    httpsvr.get("/aaa/*p", []([[maybe_unused]] http::web_request& req, http::web_response& rep) {
		rep.fill_text("aaa");
		rep.refresh();
	});

	httpsvr.post("/sss/:param/tt", []([[maybe_unused]] http::web_request& req, http::web_response& rep) {
		rep.fill_text("sss");
		rep.refresh();
	});

	httpsvr.post("/echo", []([[maybe_unused]] http::web_request& req, http::web_response& rep) {
		rep.fill_text("echo");
		rep.refresh();
	});

    // cli
    static auto httpcli = http_cli(ccfg);
	httpcli.start();

    // cli req
    httpcli.execute("GET /aaa/user/tt?name=aaa&ip=127.0.0.1 HTTP/1.1\r\n\r\n", []([[maybe_unused]]const error_code& ec, http::web_response& rep) mutable {
		if (ec) {
			std::cout << "errmsg: " << ec.message() << std::endl;
			return;
		}

		std::cout << "http client recv :" << rep.body() << std::endl;
	});

	httpcli.execute("POST /sss/user/tt?name=aaa&ip=127.0.0.1 HTTP/1.1\r\n\r\n", []([[maybe_unused]]const error_code& ec, http::web_response& rep) mutable {
		if (ec) {
			std::cout << "errmsg: " << ec.message() << std::endl;
			return;
		}

		std::cout << "http client recv :" << rep.body() << std::endl;
	});

	auto rsp = http::execute("http://127.0.0.1:18888/api/user/tt?name=aaa&ip=127.0.0.1");
	std::cout << "rsp :" << rsp.body() << std::endl;
}

#endif