/*
 * Copyright (c) 2023 bocai
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include "base/csession.hpp"

namespace bnet::base {
	template<class StreamType, class ProtoType>
	class client : public iopool
				 , public stream_ctx<StreamType> {
				 //, public std::enable_shared_from_this<client<StreamType, ProtoType>> {
	public:
        using socket_type = typename StreamType::socket_type;
		using resolver_type = typename asio::ip::basic_resolver<typename socket_type::protocol_type>;
		using endpoints_type = typename resolver_type::results_type;
		using endpoint_type = typename socket_type::lowest_layer_type::endpoint_type;
		using endpoints_iterator = typename endpoints_type::iterator;
		using session_type = csession<StreamType, ProtoType>;
		using session_ptr_type = std::shared_ptr<session_type>;
		using session_weakptr_type = std::weak_ptr<session_type>;
        using global_ctx_type = global_ctx<session_type>;
		using stream_ctx_type = stream_ctx<StreamType>;
	public:
		template<class ...Args>
		explicit client(const cli_cfg& cfg)
			: iopool(cfg.thread_num > 0 ? cfg.thread_num : std::thread::hardware_concurrency() * 2)
			, stream_ctx_type(cli_tag{})
			, cio_(iopool_.get(0))
			, globalctx_(cfg)
			, max_buffer_size_(cfg.limit_buffer_size > 0 ? cfg.limit_buffer_size : std::numeric_limits<std::size_t>::max())
		{
			this->iopool_.start();
		}

		~client() {
			this->iopool_.stop();
		}

		template<bool IsAsync = true, bool IsKeepAlive = false>
		inline bool add(std::string_view host, std::string_view port) {
			if (!is_started()) {
				return false;
			}

			clear_last_error();
            
			std::shared_ptr<session_type> session_ptr = this->make_session();
			if constexpr (IsAsync) session_ptr->template start<IsKeepAlive>(host, port, asio::detached);
			else {
				std::promise<error_code> promise;
				std::future<error_code> future = promise.get_future();
				session_ptr->template start<IsKeepAlive>(host, port,
					[&promise](std::exception_ptr ex, error_code ret) { 
					if (ex) {}
					promise.set_value(ret);
    			});
				//if (auto ret = future.wait_for(std::chrono::milliseconds(200)); ret != std::future_status::ready) {
					//	return false;
				//}
				return bool(future.get());
			}

            return true;
		}

		inline bool start() {
			estate expected = estate::stopped;
			if (!this->state_.compare_exchange_strong(expected, estate::starting)) {
				set_last_error(asio::error::already_started);
				return false;
			}

			clear_last_error();

			expected = estate::starting;
			if (!this->state_.compare_exchange_strong(expected, estate::started)) {
				set_last_error(asio::error::operation_aborted);
				return false;
			}

			for (size_t i = 0; i < globalctx_.cli_cfg_.pool_size; i++) {
				if (globalctx_.cli_cfg_.is_async) {
					add(globalctx_.cli_cfg_.host, globalctx_.cli_cfg_.port);
				}
				else {
					add<false>(globalctx_.cli_cfg_.host, globalctx_.cli_cfg_.port);
				}
			}

			return (this->is_started());
		}

		inline void stop(const error_code& ec = ec_ignore) {
			estate expected = estate::starting;
			if (this->state_.compare_exchange_strong(expected, estate::stopping))
				return this->stop_t(ec);

			expected = estate::started;
			if (this->state_.compare_exchange_strong(expected, estate::stopping))
				return this->stop_t(ec);
		}

		inline bool is_started() const {
			return (this->state_ == estate::started);
		}

		inline bool is_stopped() const {
			return (this->state_ == estate::stopped);
		}

		template<class ...Args>
		bool bind(Args&&... args) {
			return globalctx_.bind_func_->bind(std::forward<Args>(args)...);
		}

		template<class ...Args>
		bool call(Args&&... args) {
			return globalctx_.bind_func_->call(std::forward<Args>(args)...);
		}

		//广播所有session
		inline void broadcast(const std::string_view && data) {
			this->globalctx_.sessions_.foreach([&data](session_ptr_type& session_ptr) {
				session_ptr->send(data);
			});
		}

		inline session_ptr_type find_session_if(const std::function<bool(session_ptr_type&)> & fn) {
			return session_ptr_type(this->globalctx_.sessions_.find_if(fn));
		}

		inline session_ptr_type make_session() {
			auto& cio = this->iopool_.get();
#if defined(BNET_ENABLE_SSL)
			if constexpr (is_ssl_stream<StreamType>) {
				return std::make_shared<session_type>(this->globalctx_, cio, this->max_buffer_size_
					, cio, asio::ssl::stream_base::client, cio.context(), *this);
			}
#endif
			if constexpr (is_binary_stream<StreamType> || is_kcp_stream<StreamType>) {
            	return std::make_shared<session_type>(this->globalctx_, cio, this->max_buffer_size_, cio.context());
			}
		}

        //inline auto self_shared_ptr() { return this->shared_from_this(); }
	protected:
		inline void stop_t(const error_code& ec) {
            set_last_error(ec);

            this->globalctx_.sessions_.foreach([this, ec](session_ptr_type& session_ptr) {
				session_ptr->stop(ec);
			});

            estate expected = estate::stopping;
			if (this->state_.compare_exchange_strong(expected, estate::stopped)) {
				//globalctx_.bind_func_->call(Event::stop);
			}
			else
				NET_ASSERT(false);

            return;
        }

		inline auto& globalctx() { return globalctx_; }
	protected:
		nio & cio_; 
		global_ctx_type globalctx_;

		std::size_t max_buffer_size_;

		std::atomic<estate> state_ = estate::stopped;
	};
}