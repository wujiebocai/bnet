#pragma once
#include "bnet.hpp"
using namespace bnet;

//////////////////////////////http////////////////////////////////////////////////////
#if defined(BNET_ENABLE_HTTP)
template<class SvrType>
class http_svr_proxy : public SvrType {
public:
	using session_type = typename SvrType::session_type;
	using session_ptr_type = typename SvrType::session_ptr_type;
public:
	http_svr_proxy(std::size_t concurrency = std::thread::hardware_concurrency() * 2, std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)())
		: SvrType(concurrency, max_buffer_size) {

		this->bind(event::connect, []([[maybe_unused]] session_ptr_type& ptr) {
			std::cout << "http connect success" << std::endl;
		});
		this->bind(event::handshake, []([[maybe_unused]] session_ptr_type& ptr, error_code ec) {
			std::cout << "http handshake" << ec.message() << std::endl;
		});
		this->bind(event::disconnect, []([[maybe_unused]] session_ptr_type& ptr, error_code ec) {
			std::cout << "http disconnect" << ec.message() << std::endl;
		});
	
		this->get("/api/user/tt", []([[maybe_unused]] http::web_request& req, http::web_response& rep) {
			//std::cout << "server: " << req << std::endl;

			//[[maybe_unused]] auto& rep3 = rep;
			//[[maybe_unused]] auto rep2 = std::move(rep);
			//auto rep1 = rep;
			rep.fill_text("the user name is hanmeimei, .....");
			rep.refresh();

			//http::file_body::value_type aa;
			//auto a = aa;
			
			//http::message<false, http::string_body, http::fields> ss;
			//auto s = ss;
			//http::flex_body ff;
			//auto f = ff;
			//http::web_response ss;
			//auto s = ss;
		});
	}

private:
	//bnet::Timer testtimer_;
	//std::atomic<std::size_t> count_{ 1 };
};

template<class CliType>
class http_cli_proxy : public CliType {
public:
	using session_type = typename CliType::session_type;
	using session_ptr_type = typename CliType::session_ptr_type;
public:
	http_cli_proxy(std::size_t concurrency = std::thread::hardware_concurrency() * 2, std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)())
		: CliType(concurrency, max_buffer_size) {

		this->bind(event::connect, []([[maybe_unused]] session_ptr_type& ptr) {
			//const char* msg = "GET /api/user/tt HTTP/1.1\r\n\r\n";
			//ptr->send(msg);
			//ptr->send("GET /api/user/tt HTTP/1.1\r\n\r\n");
			//ptr->send(std::string("GET /sss HTTP/1.1\r\n\r\n"));
				
			/*
			base::http::web_request req;
			req.method(beast::http::verb::get);
			req.keep_alive(true);
			req.target("/del_user");
			req.body() = "Hello, world!";
			req.prepare_payload();
			ptr->send(req.base());
			*/
			std::cout << "http connect client success" << std::endl;
		});
		this->bind(event::handshake, []([[maybe_unused]]session_ptr_type& ptr, error_code ec) {
			std::cout << "http handshake client" << ec.message() << std::endl;
		});
		this->bind(event::disconnect, []([[maybe_unused]]session_ptr_type& ptr, error_code ec) {
			std::cout << "http disconnect client" << ec.message() << std::endl;
		});
		this->bind(event::recv, []([[maybe_unused]]session_ptr_type& ptr, http::web_request& req, http::web_response& rep) {
			std::ignore = req;
			// print the whole response
			std::cout << rep << std::endl;
			// print the response body
			std::cout << rep.body() << std::endl;
		});

	}
/*
	// 短链接测试接口
	void test_http_static_func() {
		//std::string_view host = "127.0.0.1";
		//std::string_view port = "8080";
		std::string_view host = "www.baidu.com";
		std::string_view port = "80";

		bnet::error_code ec;

		auto speed_rep = session_type::execute("http://speedtest-sgp1.digitalocean.com/10mb.test", std::chrono::seconds(2), ec);

		auto req1 = http::make_request("http://www.baidu.com/get_user?name=abc");

		std::cout << session_type::execute("www.baidu.com", "80", req1);
		//system("clear");
		std::cout << session_type::execute("www.baidu.com", "80", req1, ec);

		//system("clear");
		std::cout << session_type::execute("www.baidu.com", "80", req1);
		//system("clear");
		std::cout << session_type::execute("www.baidu.com", "80", req1, ec);

		std::string shost = "www.baidu.com";
		std::string sport = "80";

		//system("clear");
		std::cout << session_type::execute(shost, sport, req1);
		//system("clear");
		std::cout << session_type::execute(shost, sport, req1, ec);

		//system("clear");
		std::cout << session_type::execute(host, sport, req1);
		//system("clear");
		std::cout << session_type::execute(host, sport, req1, ec);


		//system("clear");
		std::cout << session_type::execute(shost, "80", req1);
		//system("clear");
		std::cout << session_type::execute(shost, "80", req1, ec);

		//system("clear");
		std::cout << session_type::execute(host, "80", req1);
		//system("clear");
		std::cout << session_type::execute(host, "80", req1, ec);

		//system("clear");
		std::cout << session_type::execute(shost, "80", req1);
		//system("clear");
		std::cout << session_type::execute(shost, "80", req1, ec);

		//system("clear");
		std::cout << session_type::execute(host, "80", req1);
		//system("clear");
		std::cout << session_type::execute(host, "80", req1, ec);

		auto rep1 = session_type::execute("http://www.baidu.com/get_user?name=abc", ec);
		if (ec)
			std::cout << ec.message() << std::endl;
		else
			std::cout << rep1 << std::endl;

		// GET
		auto req2 = http::make_request("GET / HTTP/1.1\r\nHost: 192.168.0.1\r\n\r\n");
		auto rep2 = session_type::execute("www.baidu.com", "80", req2, std::chrono::seconds(3), ec);
		if (ec)
			std::cout << ec.message() << std::endl;
		else
			std::cout << rep2 << std::endl;

		// POST
		auto req4 = http::make_request("POST / HTTP/1.1\r\nHost: 192.168.0.1\r\n\r\n");
		auto rep4 = session_type::execute("www.baidu.com", "80", req4, std::chrono::seconds(3), ec);
		if (ec)
			std::cout << ec.message() << std::endl;
		else
			std::cout << rep4 << std::endl;

		// POST
		http::request_t<http::string_body> req5(http::verb::post, "/", 11);
		auto rep5 = session_type::execute("www.baidu.com", "80", req5, ec);
		if (ec)
			std::cout << ec.message() << std::endl;
		else
			std::cout << rep5 << std::endl;

		// POST
		http::request_t<http::string_body> req6;
		req6.method(http::verb::post);
		req6.target("/");
		auto rep6 = session_type::execute("www.baidu.com", "80", req6, ec);
		if (ec)
			std::cout << ec.message() << std::endl;
		else
			std::cout << rep6 << std::endl;

		// POST
		http::request_t<http::string_body> req7;
		req7.method(http::verb::post);
		req7.target("/");
		req7.set(http::field::user_agent, "Chrome");
		req7.set(http::field::content_type, "text/html");
		req7.body() = "Hello World.";
		req7.prepare_payload();
		auto rep7 = session_type::execute("www.baidu.com", "80", req7, ec);
		if (ec)
			std::cout << ec.message() << std::endl;
		else
			std::cout << rep7 << std::endl;

		// convert the response body to string
		std::stringstream ss1;
		ss1 << rep7.body();
		std::cout << ss1.str() << std::endl;

		// convert the whole response to string
		std::stringstream ss2;
		ss2 << rep7;
		std::cout << ss2.str() << std::endl;


		auto path = http::url_to_path("/get_user?name=abc");
		std::cout << path << std::endl;

		auto query = http::url_to_query("/get_user?name=abc&abc=efg");
		std::cout << query << std::endl;

		std::cout << std::endl;

		//system("clear");
		std::cout << session_type::execute("http://www.baidu.com/get_user?name=abc");
		//system("clear");
		std::cout << session_type::execute("http://www.baidu.com/get_user?name=abc", ec);
		//system("clear");
		std::cout << session_type::execute("www.baidu.com", "80", "/api/get_user?name=abc");
		//system("clear");
		std::cout << session_type::execute(shost, sport, "/api/get_user?name=abc");
		//system("clear");
		std::cout << session_type::execute(host, port, "/api/get_user?name=abc");
		//system("clear");

		auto rep3 = session_type::execute("www.baidu.com", "80", "/api/get_user?name=abc", ec);
		if (ec)
			std::cout << ec.message() << std::endl;
		else
			std::cout << rep3 << std::endl;

		std::string en = http::url_encode(R"(http://www.baidu.com/json={"qeury":"name like '%abc%'","id":1})");
		std::cout << en << std::endl;
		std::string de = http::url_decode(en);
		std::cout << de << std::endl;
	}
*/
private:
	int times = 0;
};

#endif