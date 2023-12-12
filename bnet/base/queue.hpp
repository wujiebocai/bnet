#pragma once

#include <functional>
#include <chrono>
#include <deque>

namespace bnet::base {
    struct normal_queue { 
        using FunType = std::function<asio::awaitable<error_code>()>;
        normal_queue(nio& io) 
            : cio_(io)
            , timer_(io.context()) {
            timer_.expires_at(std::chrono::steady_clock::time_point::max());
        }

        ~normal_queue() = default;

        inline void enqueue(FunType&& f) {
            if (*(get_current()) == &cio_) {
                queue_.emplace_back(std::move(f));
                this->timer_.cancel_one();
                return;
            }
            asio::post(cio_.strand(), [this, f = std::move(f)]() mutable {
				queue_.emplace_back(std::move(f));
                this->timer_.cancel_one();
			});
        }

        inline asio::awaitable<void> dequeue() {
            while(!queue_.empty()) {
                if (auto ec = co_await (queue_.front())()) break;
                queue_.pop_front();
            }

            if (this->is_canceled_.test()) co_return;

            asio::error_code ec;
            co_await this->timer_.async_wait(asio::redirect_error(asio::use_awaitable, ec));

            co_return;
        }

        inline void init() { this->is_canceled_.clear(); }
        inline void uninit() { 
            this->is_canceled_.test_and_set();
            timer_.cancel(); 
        }

    protected:
        nio& cio_;
        asio::steady_timer timer_;
        std::atomic_flag is_canceled_ = ATOMIC_FLAG_INIT;
        std::deque<FunType> queue_;
    };

    struct buffer_queue {
        buffer_queue(nio& io) 
            : cio_(io)
            , timer_(io.context()) {
            timer_.expires_at(std::chrono::steady_clock::time_point::max());
        }

        ~buffer_queue() = default;

        template<class DataType>
        inline void enqueue(DataType&& data) {
            if (*(get_current()) == &cio_) {
                queue_.emplace_back(asio::buffer(std::move(data)));
                this->timer_.cancel_one();
                return;
            }
            asio::post(cio_.strand(), [this, data = asio::buffer(std::move(data))]() mutable {
				queue_.emplace_back(std::move(data));
                this->timer_.cancel_one();
			});
        }

        template<class Session>
        inline asio::awaitable<void> dequeue(Session& s) {
            while(!queue_.empty()) {
                co_await s.co_send(std::move(queue_.front()));
                queue_.pop_front();
            }

            if (this->is_canceled_.test()) co_return;

            asio::error_code ec;
            co_await this->timer_.async_wait(asio::redirect_error(asio::use_awaitable, ec));

            co_return;
        }

        inline void init() { this->is_canceled_.clear(); }
        inline void uninit() { 
            this->is_canceled_.test_and_set();
            timer_.cancel(); 
        }
    protected:
        nio& cio_;
        asio::steady_timer timer_;
        std::atomic_flag is_canceled_ = ATOMIC_FLAG_INIT;
        std::deque<asio::const_buffer> queue_;
    };
}