#include <iostream> 
#include "bnet.hpp"
#include "other.hpp"
#include <regex>
#include "tree.hpp"

#include "proxy.hpp"

using namespace bnet;

asio::io_context g_context_(1);
asio::io_context::strand g_context_s_(g_context_);
const int timenum = 10;

int64_t get_cur_time() {
	auto currentTime = std::chrono::system_clock::now();

    // Convert the current time point to milliseconds since the epoch
    auto timeSinceEpoch = currentTime.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceEpoch).count();

	return milliseconds;
}

int main() {
	net_proxy_tst(tcp, false, g_context_);

	std::cout << "test finish" << std::endl;

	asio::signal_set signals(g_context_, SIGINT, SIGTERM);
	signals.async_wait([&](auto, auto){ g_context_.stop(); });

	auto io_worker = asio::make_work_guard(g_context_);
	g_context_.run();

	return 0;
}