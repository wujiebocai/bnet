#pragma once

//#include <memory>
//#include <future>
//#include <string_view>

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
        using globalval_type = global_val<session_type>;
		using stream_ctx_type = stream_ctx<StreamType>;
	public:
		template<class ...Args>
		explicit client(std::size_t concurrency = std::thread::hardware_concurrency() * 2, std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)())
			: iopool(concurrency)
			, stream_ctx_type(cli_tag{})
			, cio_(iopool_.get(0))
			, globalval_(/*cio_*/)
			, max_buffer_size_(max_buffer_size)
			, cbfunc_(std::make_shared<CBPROXYTYPE>())
		{
			this->iopool_.start();
		}

		~client() {
			this->stop(ec_ignore);
			this->iopool_.stop();
		}

		template<bool IsAsync = true, bool IsKeepAlive = false>
		inline bool add(std::string_view host, std::string_view port) {
			if (!is_started()) {
				return false;
			}

			clear_last_error();
            
			std::shared_ptr<session_type> session_ptr = this->make_session();
			if constexpr (IsAsync) session_ptr->template start<IsKeepAlive>(host, port);
			else session_ptr->template sync_start<IsKeepAlive>(host, port);

            return true;
		}

		inline bool start() {
			estate expected = estate::stopped;
			if (!this->state_.compare_exchange_strong(expected, estate::starting)) {
				set_last_error(asio::error::already_started);
				return false;
			}

			clear_last_error();

			//cbfunc_->call(Event::init);

			expected = estate::starting;
			if (!this->state_.compare_exchange_strong(expected, estate::started)) {
				set_last_error(asio::error::operation_aborted);
				return false;
			}

			return (this->is_started());
		}

		inline void stop(const error_code& ec) {
			estate expected = estate::starting;
			if (this->state_.compare_exchange_strong(expected, estate::stopping))
				asio::co_spawn(this->cio_.context(), [this/*, self = self_shared_ptr()*/, ec = std::move(ec)]() { 
				    return this->stop_t(ec);
			    }, asio::detached);

			expected = estate::started;
			if (this->state_.compare_exchange_strong(expected, estate::stopping))
				asio::co_spawn(this->cio_.context(), [this/*, self = self_shared_ptr()*/, ec = std::move(ec)]() { 
				    return this->stop_t(ec);
			    }, asio::detached);
		}

        inline asio::awaitable<void> stop_t(const error_code& ec) {
            set_last_error(ec);

            this->globalval_.sessions_.foreach([this, ec](session_ptr_type& session_ptr) {
				session_ptr->stop(ec);
			});

            estate expected = estate::stopping;
			if (this->state_.compare_exchange_strong(expected, estate::stopped)) {
				//cbfunc_->call(Event::stop);
			}
			else
				NET_ASSERT(false);

            co_return;
        }

		inline session_ptr_type make_session() {
			auto& cio = this->iopool_.get();
#if defined(NET_USE_SSL)
			if constexpr (is_ssl_stream<StreamType>) {
				return std::make_shared<session_type>(this->globalval_, this->cbfunc_, cio, this->max_buffer_size_
					, cio, asio::ssl::stream_base::client, cio.context(), *this);
			}
#endif
/*
			if constexpr (is_binary_streamtype_v<STREAMTYPE> || is_ws_streamtype_v<STREAMTYPE>) {
				return std::make_shared<session_type>(this->sessions_, this->cbfunc_, cio, this->max_buffer_size_
					, cio.context());
			}
			if constexpr (is_kcp_streamtype_v<STREAMTYPE>) {
				return std::make_shared<session_type>(this->sessions_, this->cbfunc_, cio, this->max_buffer_size_, cio, cio.context());
			}
*/
			if constexpr (is_binary_stream<StreamType> || is_kcp_stream<StreamType>) {
            	return std::make_shared<session_type>(this->globalval_, this->cbfunc_, cio, this->max_buffer_size_, cio.context());
			}
		}

		inline bool is_started() const {
			return (this->state_ == estate::started);
		}

		inline bool is_stopped() const {
			return (this->state_ == estate::stopped);
		}

		template<class ...Args>
		bool bind(Args&&... args) {
			return cbfunc_->bind(std::forward<Args>(args)...);
		}

		template<class ...Args>
		bool call(Args&&... args) {
			return cbfunc_->call(std::forward<Args>(args)...);
		}

		//广播所有session
		inline void broadcast(const std::string_view && data) {
			this->globalval_.sessions_.foreach([&data](session_ptr_type& session_ptr) {
				session_ptr->send(data);
			});
		}

		inline session_ptr_type find_session_if(const std::function<bool(session_ptr_type&)> & fn) {
			return session_ptr_type(this->globalval_.sessions_.find_if(fn));
		}

        //inline auto self_shared_ptr() { return this->shared_from_this(); }

		inline auto& globalval() { return globalval_; }
	protected:
		nio & cio_; 
		globalval_type globalval_;

		std::size_t max_buffer_size_;

		func_proxy_imp_ptr cbfunc_;

		std::atomic<estate> state_ = estate::stopped;
	};
}