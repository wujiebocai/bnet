#pragma once
#include "bnet.hpp"
using namespace bnet;

#if defined(BNET_ENABLE_HTTP)
void http_tst() {
    // svr
  	static auto httpsvr = http_svr(6);
  	httpsvr.start("0.0.0.0", "18888");
    // route
    httpsvr.get("/api/*/tt", []([[maybe_unused]] http::web_request& req, http::web_response& rep) {
		rep.fill_text("hello world...");
		rep.refresh();
	});

	httpsvr.post("/echo", []([[maybe_unused]] http::web_request& req, http::web_response& rep) {
		rep.fill_text("hello world...");
		rep.refresh();
	});

    // cli
    static auto httpcli = http_cli(3);
	httpcli.start();
    // add connection pool
	for (int i = 0; i < 100; ++i) {
		httpcli.add<false>("192.168.39.63", "18888");
	}

    // cli req
    httpcli.execute("GET /api/user/tt?name=aaa&ip=127.0.0.1 HTTP/1.1\r\n\r\n", []([[maybe_unused]]const error_code& ec, http::web_response& rep) mutable {
		if (ec) {
			std::cout << "errmsg: " << ec.message() << std::endl;
		}

		std::cout << "http client recv :" << rep << std::endl;
	});

	auto rsp = http::execute("http://127.0.0.1:18888/api/user/tt?name=aaa&ip=127.0.0.1");
	std::cout << "rsp :" << rsp << std::endl;
}

#endif