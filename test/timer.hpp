#pragma once
#include "bnet.hpp"
using namespace bnet;

void timer_tst() {
    asio::io_context context(1);
    bnet::Timer timer(context);
    
	// loop call
	timer.start(std::chrono::seconds(5), [](std::error_code& ec) mutable {
		if (ec) {
			std::cerr << "err: " << ec.message() << std::endl;
		}

		std::cout << "timer loop cb" << std::endl;

		// return false， exit；return true， continue
	});

    auto io_worker = asio::make_work_guard(context);
	context.run();
}

void timer_tst1() {
	asio::io_context context(1);
    bnet::Timer timer(context);

	// once call
    timer.start<false>(std::chrono::seconds(5), [](std::error_code& ec) mutable {
        if (ec) {
			std::cerr << "err: " << ec.message() << std::endl;
		}

		std::cout << "timer after cb" << std::endl;
    });

    auto io_worker = asio::make_work_guard(context);
	context.run();
}