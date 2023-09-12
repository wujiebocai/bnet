#include <iostream> 
#include "bnet.hpp"
#include "other.hpp"
#include "httptmp.hpp"
#include <regex>
#include "tree_tst.hpp"

using namespace bnet;

asio::io_context g_context_(1);
asio::io_context::strand g_context_s_(g_context_);
const int timenum = 10;

void print_time() {
	// 获取当前时间
    time_t currentTime = time(0);

    // 将当前时间转换为本地时间
    struct tm* localTime = localtime(&currentTime);

    // 显示当前时间
    std::cout << "当前时间为: "
              << localTime->tm_year + 1900 << "-"
              << localTime->tm_mon + 1 << "-"
              << localTime->tm_mday << " "
              << localTime->tm_hour << ":"
              << localTime->tm_min << ":"
              << localTime->tm_sec << std::endl;
}

int64_t get_cur_time() {
	auto currentTime = std::chrono::system_clock::now();

    // Convert the current time point to milliseconds since the epoch
    auto timeSinceEpoch = currentTime.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceEpoch).count();

	return milliseconds;
}

template<class SvrType>
class svr_proxy : public SvrType {
public:
	using session_ptr_type = typename SvrType::session_ptr_type;
public:
	svr_proxy(std::size_t concurrency = std::thread::hardware_concurrency() * 2, std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)()) 
		: SvrType(concurrency, max_buffer_size)
		, ctimer_(g_context_, asio::chrono::seconds(0)) {

		this->bind(event::connect, [&]([[maybe_unused]]session_ptr_type& ptr) {
			std::cout << "connect success" << ", " << this->session_count() << std::endl;
			//ptr->stop(ec);
		});
		this->bind(event::handshake, []([[maybe_unused]]session_ptr_type& ptr, error_code ec) { // only ssl call back
			std::cout << "handshake" << ec.message() << std::endl;
		});
		this->bind(event::disconnect, []([[maybe_unused]]session_ptr_type& ptr, error_code ec) {
			std::cout << "disconnect" << ec.message() << std::endl;
		});
		this->bind(event::recv, [&]([[maybe_unused]]session_ptr_type& ptr, std::string_view&& s) {
			//std::cout << s << count_ << std::endl;
			//count_.fetch_add(s.size());
			ptr->send(std::move(s));
			++count_;
		});

		this->bind(event::upgrade, &svr_proxy::upgrade, this);
	}

	inline void upgrade([[maybe_unused]]session_ptr_type& ptr, error_code ec) {
		std::cout << "upgrade" << ec.message() << std::endl;
	}

	inline void test_timer() {
		asio::co_spawn(g_context_, [this]() { 
			return this->count_timer();
		}, asio::detached);
	}

    inline asio::awaitable<void> count_timer() {
        //asio::steady_timer timer(co_await asio::this_coro::executor);
        
        while (true) {
            //this->acceptor_timer_.expires_after(std::chrono::seconds(10));
			//ctimer_.expires_after(ctimer_.expiry() + std::chrono::seconds(10));
			ctimer_.expires_at(ctimer_.expiry() + asio::chrono::seconds(timenum));
            co_await ctimer_.async_wait(asio::use_awaitable);

			print_time();

            std::cout << count_ / timenum << " " << count_ << std::endl;
		    count_ = 0;
        }
    }

private:
	asio::steady_timer ctimer_;
	//bnet::Timer testtimer_;
	std::atomic<std::size_t> count_{ 0 };
};

template<class CliType>
class cli_proxy : public CliType {
public:
	using session_ptr_type = typename CliType::session_ptr_type;
public:
	cli_proxy(std::size_t concurrency = std::thread::hardware_concurrency() * 2, std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)())
		: CliType(concurrency, max_buffer_size) {

		this->bind(event::connect, []([[maybe_unused]]session_ptr_type& ptr) {
			std::string msgdata(1024, 'a');
			
			ptr->send("a");
				
			/*
			base::http::web_request req;
			req.method(beast::http::verb::get);
			req.keep_alive(true);
			req.target("/del_user");
			req.body() = "Hello, world!";
			req.prepare_payload();
			ptr->send(req.base());
			*/
			std::cout << "client connect success" << std::endl;
		});
		this->bind(event::handshake, []([[maybe_unused]]session_ptr_type& ptr, error_code ec) { // only ssl call back
			std::cout << "client handshake" << ec.message() << std::endl;
		});
		this->bind(event::disconnect, []([[maybe_unused]]session_ptr_type& ptr, error_code ec) {
			std::cout << "client disconnect" << ec.message() << std::endl;
			if (ec) {
				//ptr->reconn(); //测试异常退出重连
			}
			//ptr->reconn();
		});
		this->bind(event::recv, []([[maybe_unused]]session_ptr_type& ptr, std::string_view&& s) {
			ptr->send(std::move(s));
		});
	}
};

void tcp_tst() {
#if 1
	// svr
	static auto tcpsvr = svr_proxy<tcp_svr>(8);
	tcpsvr.start("0.0.0.0", "8585");
    tcpsvr.test_timer();
	//std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    //cli
    static auto tcpcli = cli_proxy<tcp_cli>(5);
    tcpcli.start();
    for (int i = 0; i < 44; ++i) {
        tcpcli.add("192.168.152.62", "8585");
    }
	//std::this_thread::sleep_for(std::chrono::milliseconds(2000));
#endif
}

void udp_tst() {
#if 1
	// svr
	static auto udp_svr_ptr = std::make_shared<svr_proxy<udp_svr>>(8);
	udp_svr_ptr->start("0.0.0.0", "8585");
    udp_svr_ptr->test_timer();
    //cli
    static auto udp_cli_ptr = std::make_shared<cli_proxy<udp_cli>>(5);
    udp_cli_ptr->start();
    for (int i = 0; i < 44; ++i) {
        udp_cli_ptr->add("127.0.0.1", "8585");
    }
#endif
}

void kcp_tst() {
#if 1
	// svr
	static auto kcp_svr_ptr = std::make_shared<svr_proxy<kcp_svr>>(8);
	kcp_svr_ptr->start("0.0.0.0", "8585");
    kcp_svr_ptr->test_timer();
    //cli
    static auto kcp_cli_ptr = std::make_shared<cli_proxy<kcp_cli>>(5);
    kcp_cli_ptr->start();
    for (int i = 0; i < 44; ++i) {
        kcp_cli_ptr->add("127.0.0.1", "8585");
    }
#endif
}

std::atomic<std::size_t> httpcount{ 0 };
void http_tst() {
#if defined(BNET_ENABLE_HTTP)
	// svr
  	static auto http_svr_ptr = std::make_shared<http_svr_proxy<http_svr>>(6);
  	http_svr_ptr->start("0.0.0.0", "18888");
	//http_svr_ptr->test_timer();
	//cli
	static auto http_cli_ptr = std::make_shared<http_cli_proxy<http_cli>>(3);
	http_cli_ptr->start();
	for (int i = 0; i < 1; ++i) {
		http_cli_ptr->add<false>("192.168.39.63", "9902");
	}
	auto start_time = get_cur_time();
	http_cli_ptr->execute("GET /check?name=aaa&ip=127.0.0.1 HTTP/1.1\r\n\r\n", [start_time]([[maybe_unused]]const error_code& ec, http::web_response& rep) mutable {
		if (ec) {
			std::cout << "errmsg: " << ec.message() << std::endl;
		}

		std::cout << "client:" << rep << std::endl;
	});

	auto rsp = http::execute("http://192.168.39.63:9902/check?name=aaa&ip=127.0.0.1");
	std::cout << "sssssssssssssssss:" << rsp << std::endl;
	return;
	std::string_view msg("GET /api/user/tt HTTP/1.1\r\n\r\n");
	for (int i = 0; i < 100000; ++i) {
	http_cli_ptr->execute(msg, [start_time]([[maybe_unused]]const error_code& ec, http::web_response& rep) mutable {
		//std::cout << "client:" << rep << std::endl;
		if (ec) {
			std::cout << "errmsg: " << ec.message() << std::endl;
		}

		httpcount.fetch_add(1);
		if (httpcount.load() == 300000) {
			auto end_time = get_cur_time();
			std::cout << "======================================================= " << httpcount << " " << (end_time - start_time) << std::endl;
		}
	});
	http_cli_ptr->execute(msg, [start_time]([[maybe_unused]]const error_code& ec, [[maybe_unused]]http::web_response& rep) {
		//std::cout << "client2:" << rep << std::endl;
		if (ec) {
			std::cout << "errmsg: " << ec.message() << std::endl;
		}

		httpcount.fetch_add(1);
		if (httpcount.load() == 300000) {
			auto end_time = get_cur_time();
			std::cout << "======================================================= " << httpcount << " " << (end_time - start_time) << std::endl;
		}
	});

	http_cli_ptr->execute(msg, [start_time]([[maybe_unused]]const error_code& ec, [[maybe_unused]]http::web_response& rep) {
		//std::cout << "client3:" << rep << std::endl;
		//std::cout << "client3:" << rep.body().text() << std::endl;
		if (ec) {
			std::cout << "errmsg: " << ec.message() << std::endl;
		}

		httpcount.fetch_add(1);
		if (httpcount.load() == 300000) {
			auto end_time = get_cur_time();
			std::cout << "======================================================= " << httpcount << " " << (end_time - start_time) << std::endl;
		}
	});
	}
#endif
}

void ws_tst() {
#if defined(BNET_ENABLE_HTTP)
	// svr
  	static auto ws_svr_ptr = std::make_shared<svr_proxy<ws_svr>>(8);
  	ws_svr_ptr->start("0.0.0.0", "8888");
	ws_svr_ptr->test_timer();
	//cli
	static auto ws_cli_ptr = std::make_shared<cli_proxy<ws_cli>>(5);
	ws_cli_ptr->start();
	for (int i = 0; i < 5000; ++i) {
		ws_cli_ptr->add("127.0.0.1", "8888");
	}
#endif
}

void wss_tst() {
#if defined(BNET_ENABLE_SSL) && defined(BNET_ENABLE_HTTP)
	// svr
	static auto wss_svr_ptr = std::make_shared<svr_proxy<wss_svr>>(8);
    wss_svr_ptr->set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);
    auto ec = wss_svr_ptr->set_cert_buffer(ca_crt, server_crt, server_key, "123456");
    if (ec) {
        std::cout << "server:" << ec.message() << std::endl;
        return;
    }
    ec = wss_svr_ptr->set_dh_buffer(dh);
    if (ec) {
        std::cout << "server:" << ec.message() << std::endl;
        return;
    }
    wss_svr_ptr->start("0.0.0.0", "8888");
    wss_svr_ptr->test_timer();
	
    // cli
    static auto wss_cli_ptr = std::make_shared<cli_proxy<wss_cli>>(5);
    wss_cli_ptr->set_verify_mode(asio::ssl::verify_peer);
    ec = wss_cli_ptr->set_cert_buffer(ca_crt, client_crt, client_key, "123456");
    wss_cli_ptr->start();
    if (ec) {
        std::cout << "server:" << ec.message() << std::endl;
        return;
    }
    for (int i = 0; i < 100; ++i) {
        wss_cli_ptr->add("127.0.0.1", "8888");
    }
#endif
}

void timer_tst() {
	int count = 0;
	static bnet::Timer timer(g_context_);
	timer.start(std::chrono::seconds(5), [count](std::error_code& ec) mutable {
		if (ec) {
			std::cerr << "err: " << ec.message() << std::endl;
		}
		print_time();

		count++;
		if (count >= 3) {
			timer.stop();
		}
	});
}

void route_tst() {
	auto start_time = get_cur_time();

  	//for (int i = 0; i < 100; i++) {
    	test_case_insensitive_route();
  	//}

  	auto end_time1 = get_cur_time();
	std::cout << "Path 1 matches the route. cost time:" << (end_time1 - start_time) << std::endl;
}

//int main(int argc, char * argv[]) {
int main() {
	http_tst();
	std::cout << "ssssssssssssssstop" << std::endl;

	asio::signal_set signals(g_context_, SIGINT, SIGTERM);
	signals.async_wait([&](auto, auto){ g_context_.stop(); });

	auto io_worker = asio::make_work_guard(g_context_);
	g_context_.run();

	return 0;
}