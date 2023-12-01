#pragma once

#include <functional>
#include <chrono>
#include <deque>

namespace bnet::base {
    struct normal_queue { 
        using FunType = std::function<asio::awaitable<size_t>()>;
        normal_queue(nio& io) 
            : cio_(io)
            , timer_(io.context()) {
            timer_.expires_at(std::chrono::steady_clock::time_point::max());
        }

        ~normal_queue() {
            timer_.cancel();
        }

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
                co_await (queue_.front())();
                queue_.pop_front();
            }

            asio::error_code ec;
            co_await this->timer_.async_wait(asio::redirect_error(asio::use_awaitable, ec));

            co_return;
        }

    protected:
        nio& cio_;
        asio::steady_timer timer_;
        std::deque<std::function<asio::awaitable<size_t>()>> queue_;
    };

    struct buffer_queue {
        buffer_queue(nio& io) 
            : cio_(io)
            , timer_(io.context()) {
            timer_.expires_at(std::chrono::steady_clock::time_point::max());
        }

        ~buffer_queue() {
            timer_.cancel();
        }

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

            asio::error_code ec;
            co_await this->timer_.async_wait(asio::redirect_error(asio::use_awaitable, ec));

            co_return;
        }

    protected:
        nio& cio_;
        asio::steady_timer timer_;
        std::deque<asio::const_buffer> queue_;
    };
}