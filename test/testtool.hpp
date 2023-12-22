#pragma once

#include <iostream> 
#include "bnet.hpp"

void print_time() {
    time_t currentTime = time(0);
    tm* localTime = localtime(&currentTime);
    std::cout << "当前时间为: "
              << localTime->tm_year + 1900 << "-"
              << localTime->tm_mon + 1 << "-"
              << localTime->tm_mday << " "
              << localTime->tm_hour << ":"
              << localTime->tm_min << ":"
              << localTime->tm_sec << std::endl;
}

class tst_tool {
public:
    inline void add_count() {
        count_.fetch_add(1, std::memory_order::relaxed);
    }
    
    inline void test_timer(asio::io_context& io) {
		asio::co_spawn(io, count_timer(io), asio::detached);
	}

    inline asio::awaitable<void> count_timer(asio::io_context& io) {
        asio::steady_timer ctimer_(io, asio::chrono::seconds(0));
        while (true) {
            //ctimer_.expires_after(std::chrono::seconds(timenum_));
			ctimer_.expires_at(ctimer_.expiry() + asio::chrono::seconds(timenum_));
            co_await ctimer_.async_wait(asio::use_awaitable);

			print_time();

            std::cout << "qps: " << count_ / timenum_ << std::endl;
		    count_ = 0;
        }
    }

private:
	std::atomic<std::size_t> count_{ 0 };
    const int timenum_ = 10;
};