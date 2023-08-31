#pragma once

//#include <memory>
#include <string_view>
#include "tool/func_proxy.hpp"
#include "tool/bytebuffer.hpp"
#include "tool/util.hpp"
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
		using globalval_type = global_val<session_type>;
	public:
		explicit server(std::size_t concurrency = std::thread::hardware_concurrency() * 2, std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)())
			: iopool(concurrency)
			, stream_ctx_type(svr_tag{})
			, acceptor_type(iopool_.get(0))
            , accept_io_(iopool_.get(0))
			, globalval_(/*accept_io_*/)
			, max_buffer_size_(max_buffer_size)
			, cbfunc_(std::make_shared<CBPROXYTYPE>())
			
		{
			this->iopool_.start();
		}

		~server() {
			//this->stop(ec_ignore);
			this->iopool_.stop();
		}

        inline void start(const std::string_view& host, const std::string_view& service) {
			asio::co_spawn(this->accept_io_.context(), [this/*, self = this->self_shared_ptr()*/, host, service]() { 
				return this->start_t(host, service);
			}, asio::detached);
        }

		inline void stop(const error_code& ec) {
			asio::co_spawn(this->accept_io_.context(), [this/*, self = this->self_shared_ptr()*/, ec = std::move(ec)]() { 
				return this->stop_t(ec);
			}, asio::detached);
		}

	//protected:
		inline asio::awaitable<bool> start_t(std::string_view host, std::string_view service) {
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

				//cbfunc_->call(event::init);

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

		inline asio::awaitable<bool> stop_t(const error_code& ec) {
			try {
				estate expected_starting = estate::starting;
				estate expected_started = estate::started;
				if (this->state_.compare_exchange_strong(expected_starting, estate::stopping) || 
					this->state_.compare_exchange_strong(expected_started, estate::stopping)) {
					set_last_error(ec);

					this->acceptor_stop();

					this->globalval_.sessions_.foreach([this, ec](session_ptr_type & session_ptr) {
						session_ptr->stop(ec);
					});

					estate expected_stopping = estate::stopping;
					if (this->state_.compare_exchange_strong(expected_stopping, estate::stopped)) {
						//cbfunc_->call(event::stop);
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
			this->globalval_.sessions_.foreach([&data](session_ptr_type& session_ptr) {
				session_ptr->send(data);
			});
		}

		inline session_ptr_type make_session() {
			auto& cio = this->iopool_.get();
#if defined(NET_USE_SSL)
			if constexpr (is_ssl_stream<StreamType>) {
				return std::make_shared<session_type>(this->globalval_, this->cbfunc_, cio, this->max_buffer_size_
					, cio, asio::ssl::stream_base::server, cio.context(), *this);
			}
#endif
			if constexpr (is_binary_stream<StreamType> || is_kcp_stream<StreamType>) {
				if constexpr (is_udp_by_stream<StreamType>) {
					return std::make_shared<session_type>(this->globalval_, this->cbfunc_, this->cio_, this->max_buffer_size_, this->acceptor_);
				}

				if constexpr (is_tcp_by_stream<StreamType>) {
					return std::make_shared<session_type>(this->globalval_, this->cbfunc_, cio, this->max_buffer_size_, cio.context());
				}
			}
		}

		inline std::size_t session_count() { return this->globalval_.sessions_.size(); }

		inline void foreach_session(const std::function<void(session_ptr_type&)> & fn) {
			this->globalval_.sessions_.foreach(fn);
		}

		inline session_ptr_type find_session_if(const std::function<bool(session_ptr_type&)> & fn) {
			return session_ptr_type(this->globalval_.sessions_.find_if(fn));
		}

		template<class ...Args>
		bool bind(Args&&... args) {
			return cbfunc_->bind(std::forward<Args>(args)...);
		}

		template<class ...Args>
		bool call(Args&&... args) {
			return cbfunc_->call(std::forward<Args>(args)...);
		}

		auto& get_iopool() { return iopool_; }
		auto& get_sessions() { return this->globalval_.sessions_; }

        //inline auto self_shared_ptr() { return this->shared_from_this(); }
		
		inline auto& globalval() { return globalval_; }
	protected:
		nio & accept_io_;

		globalval_type globalval_;

		std::atomic<estate> state_ = estate::stopped;

		std::size_t max_buffer_size_ = 0;
		std::size_t min_buffer_size_ = 0;

		func_proxy_imp_ptr cbfunc_;
	};
}
