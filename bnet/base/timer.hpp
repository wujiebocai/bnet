#pragma once

//#include <asio/any_completion_handler.hpp>
//#include <asio/any_io_executor.hpp>
//#include <asio/async_result.hpp>
//#include <asio/error.hpp>
//#include <asio/consign.hpp>
//#include <asio/steady_timer.hpp>
//#include <chrono>

namespace bnet::base {
	void async_sleep_impl(
        asio::any_completion_handler<void(std::error_code)> handler,
        asio::any_io_executor ex, std::chrono::nanoseconds duration) {
        auto timer = std::make_shared<asio::steady_timer>(ex, duration);
        timer->async_wait(asio::consign(std::move(handler), timer));
    }

    template <typename CompletionToken>
    inline auto async_sleep(asio::any_io_executor ex,
        std::chrono::nanoseconds duration, CompletionToken&& token) {
        return asio::async_initiate<CompletionToken, void(std::error_code)>(
        async_sleep_impl, token, std::move(ex), duration);
    }

    template<class Fn>
    concept is_timer_func = std::is_invocable_r<void, Fn, std::error_code&>::value || std::is_invocable_r<bool, Fn, std::error_code&>::value;

    class Timer {
	public:
		explicit Timer(asio::io_context& ctx)
			: ctx_(ctx)
            , timer_(ctx, std::chrono::milliseconds(0)) {
		}

		~Timer() {
            this->stop();
		}

	public:
        template<bool IsLoop = true, is_timer_func Fn, class Rep, class Period>
		inline void start(std::chrono::duration<Rep, Period> interval, Fn&& f) {
            if constexpr (std::is_void_v<Fn>) {
                return;
            }

            if (interval > std::chrono::duration_cast<
				std::chrono::duration<Rep, Period>>((asio::steady_timer::duration::max)()))
				interval = std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(
					(asio::steady_timer::duration::max)());

            //this->timer_canceled_.clear();
			asio::co_spawn(this->ctx_, [this, interval = std::move(interval), f = std::move(f)]() mutable { 
				return this->template start_t<IsLoop>(interval, f);
			}, asio::detached);
		}

        template<bool IsLoop = true, is_timer_func Fn, class Rep, class Period>
        inline asio::awaitable<void> start_t(std::chrono::duration<Rep, Period> interval, Fn&& f) {
            while (IsLoop) {
			    timer_.expires_at(timer_.expiry() + interval);
                //co_await timer_.async_wait(asio::use_awaitable);
                std::error_code ec;
          		co_await timer_.async_wait(asio::redirect_error(asio::use_awaitable, ec));

                if (this->timer_canceled_.test()) {
                    co_return;
                }

                constexpr bool isvoid = std::is_same_v<decltype(f(ec)), void>;
			    if constexpr (isvoid) {
                    f(ec);
                }
                else {
                    if (auto ret = f(ec); !ret) {
                        co_return;
                    } 
                }
            }
        }

		inline void stop() {
			this->timer_canceled_.test_and_set();
			this->timer_.cancel();
		}

	protected:
		asio::io_context& ctx_;

		asio::steady_timer timer_;

		std::atomic_flag timer_canceled_ = ATOMIC_FLAG_INIT;

		//std::chrono::milliseconds interval_ = std::chrono::minutes(60);

		//std::chrono::time_point<std::chrono::system_clock> active_time_ = std::chrono::system_clock::now();
	};
}
