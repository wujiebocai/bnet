/*
 * Copyright (c) 2023 bocai
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

//#include <memory>
#include <string_view>
#include "tool/func_proxy.hpp"
#include "tool/bytebuffer.hpp"
#include "tool/util.hpp"
#include "tool/function_traits.hpp"
#include "base/define.hpp"
#include "base/error.hpp"
#include "base/helper.hpp"
#include "base/iopool.hpp"
#include "base/stream.hpp"
#include "base/proto.hpp"
#include "base/acceptor.hpp"
#include "base/global.hpp"
#include "base/session.hpp"

namespace bnet::base {
	template<class StreamType, class ProtoType>
	class server : public iopool
				 , public stream_ctx<StreamType>
				 , public acceptor<server<StreamType, ProtoType>, StreamType> {
				// , public std::enable_shared_from_this<server<StreamType, ProtoType>> {
	public:
		using server_type = server<StreamType, ProtoType>;
		using session_type = session<StreamType, ProtoType>;
		using session_ptr_type = std::shared_ptr<session_type>;
		using session_weakptr_type = std::weak_ptr<session_type>;
		using acceptor_type = acceptor<server, StreamType>;
		using stream_ctx_type = stream_ctx<StreamType>;
		using global_ctx_type = global_ctx<session_type>;
	public:
		explicit server(const svr_cfg& cfg)
			: iopool(cfg.thread_num > 0 ? cfg.thread_num : std::thread::hardware_concurrency() * 2)
			, stream_ctx_type(svr_tag{})
			, acceptor_type(iopool_.get(0))
            , accept_io_(iopool_.get(0))
			, globalctx_(cfg)
			, max_buffer_size_(cfg.limit_buffer_size > 0 ? cfg.limit_buffer_size : std::numeric_limits<std::size_t>::max())
			
		{
			this->iopool_.start();
		}

		~server() {
			this->stop();
			this->iopool_.stop();
		}

        inline void start() {
			asio::co_spawn(this->accept_io_.context(), co_start(globalctx_.svr_cfg_.host, globalctx_.svr_cfg_.port), asio::detached);
        }

		inline void stop(const error_code& ec = ec_ignore) {
			asio::co_spawn(this->accept_io_.context(), co_stop(ec), asio::detached);
		}

		inline asio::awaitable<bool> co_start(std::string_view host, std::string_view service) {
			try {
				clear_last_error();
				
				estate expected = estate::stopped;
				if (!this->state_.compare_exchange_strong(expected, estate::starting)) {
					asio::detail::throw_error(asio::error::already_started);
				}

				auto ec = co_await this->acceptor_start(host, service);
				if (ec) {
					asio::detail::throw_error(ec);
				}

				expected = estate::starting;
				if (!this->state_.compare_exchange_strong(expected, estate::started)) {
					asio::detail::throw_error(asio::error::operation_aborted);
				}

				co_return (this->is_started());
			}
			catch (system_error & e) {
				set_last_error(e);
				this->stop(e.code());
			}

			co_return false;
		}

		inline asio::awaitable<bool> co_stop(const error_code& ec) {
			try {
				estate expected_starting = estate::starting;
				estate expected_started = estate::started;
				if (this->state_.compare_exchange_strong(expected_starting, estate::stopping) || 
					this->state_.compare_exchange_strong(expected_started, estate::stopping)) {
					set_last_error(ec);

					this->acceptor_stop();

					this->globalctx_.sessions_.foreach([this, ec](session_ptr_type & session_ptr) {
						session_ptr->stop(ec);
					});

					estate expected_stopping = estate::stopping;
					if (this->state_.compare_exchange_strong(expected_stopping, estate::stopped)) {
						//globalctx_.bind_func_->call(event::stop);
					}
					else
						NET_ASSERT(false);
					
					co_return true;
				}
			}
			catch (system_error & e) {
				set_last_error(e);
			}

			co_return false;
		}

		inline bool is_started() const {
			return (this->state_ == estate::started && this->is_open());
		}

		inline bool is_stopped() const {
			return (this->state_ == estate::stopped && !this->is_open());
		}

		inline void broadcast(const std::string_view && data) {
			this->globalctx_.sessions_.foreach([&data](session_ptr_type& session_ptr) {
				session_ptr->send(data);
			});
		}

		inline session_ptr_type make_session() {
			auto& cio = this->iopool_.get();
#if defined(BNET_ENABLE_SSL)
			if constexpr (is_ssl_stream<StreamType>) {
				return std::make_shared<session_type>(this->globalctx_, cio, this->max_buffer_size_
					, cio, asio::ssl::stream_base::server, cio.context(), *this);
			}
#endif
			if constexpr (is_binary_stream<StreamType> || is_kcp_stream<StreamType>) {
				if constexpr (is_udp_by_stream<StreamType>) {
					return std::make_shared<session_type>(this->globalctx_, this->cio_, this->max_buffer_size_, this->acceptor_);
				}

				if constexpr (is_tcp_by_stream<StreamType>) {
					return std::make_shared<session_type>(this->globalctx_, cio, this->max_buffer_size_, cio.context());
				}
			}
		}

		inline std::size_t session_count() { return this->globalctx_.sessions_.size(); }

		inline void foreach_session(const std::function<void(session_ptr_type&)> & fn) {
			this->globalctx_.sessions_.foreach(fn);
		}

		inline session_ptr_type find_session_if(const std::function<bool(session_ptr_type&)> & fn) {
			return session_ptr_type(this->globalctx_.sessions_.find_if(fn));
		}

		template<class ...Args>
		bool bind(Args&&... args) {
			return globalctx_.bind_func_->bind(std::forward<Args>(args)...);
		}

		template<class ...Args>
		bool call(Args&&... args) {
			return globalctx_.bind_func_->call(std::forward<Args>(args)...);
		}

		auto& get_iopool() { return iopool_; }
		auto& get_sessions() { return this->globalctx_.sessions_; }

        //inline auto self_shared_ptr() { return this->shared_from_this(); }
		
		inline auto& globalctx() { return globalctx_; }
	protected:
		nio & accept_io_;

		global_ctx_type globalctx_;

		std::atomic<estate> state_ = estate::stopped;

		std::size_t max_buffer_size_ = 0;
		std::size_t min_buffer_size_ = 0;
	};
}
