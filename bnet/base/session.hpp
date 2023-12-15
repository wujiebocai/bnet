/*
 * Copyright (c) 2023 bocai
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include "base/transfer.hpp"

namespace bnet::base {
	template<class StreamType, class ProtoType>
	class session : public StreamType
				  , public ProtoType
				  , public transfer<session<StreamType, ProtoType>, StreamType, ProtoType, svr_tag>
				  , public std::enable_shared_from_this<session<StreamType, ProtoType>> {
	public:
		using session_type = session<StreamType, ProtoType>;
		using session_ptr_type = std::shared_ptr<session_type>;
		using stream_type = StreamType;
		using proto_type = ProtoType;
		using transfer_type = transfer<session, StreamType, ProtoType, svr_tag>;
		using global_ctx_type = global_ctx<session_type>;
		//using key_type = typename std::conditional<is_udp_socket_v<SocketType>, asio::ip::udp::endpoint, std::size_t>::type;
		using key_type = std::size_t;
	public:
		template<class ...Args>
		explicit session(global_ctx_type& globalctx, nio & io,
						std::size_t max_buffer_size, Args&&... args)
			: stream_type(std::forward<Args>(args)...)
			, proto_type(max_buffer_size)
			, transfer_type(io, max_buffer_size)
			, cio_(io)
			, globalctx_(globalctx)
		{
		
		}

		~session() = default;

		template<bool iskeepalive = false>
		inline void start() {
			asio::co_spawn(this->cio_.context(), [this, self = self_shared_ptr()]() { 
				return self->template co_start<iskeepalive>();
			}, asio::detached);
		}

		inline void stop(const error_code& ec = ec_ignore) {
			asio::co_spawn(this->cio_.context(), [this, self = self_shared_ptr(), ec = std::move(ec)]() { 
				return self->co_stop(ec);
			}, asio::detached);
		}

		inline bool is_started() const {
			return (this->state_ == estate::started && this->socket().lowest_layer().is_open());
		}
		inline bool is_stopped() const {
			return (this->state_ == estate::stopped && !this->socket().lowest_layer().is_open());
		}

		inline key_type hash_key() const {
			if constexpr (is_udp_by_stream<StreamType>) {
				return std::hash<asio::ip::udp::endpoint>()(this->remote_endpoint_);
			}

			return reinterpret_cast<key_type>(this);
		}

		//imp
		inline auto self_shared_ptr() { return this->shared_from_this(); }
		inline nio& cio() { return cio_; }
		//inline void set_first_pack(std::string&& str) { first_pack_ = std::move(str); }
		//inline auto& get_first_pack() { return first_pack_; }
		inline dynamic_buffer<>& rbuffer() { return rbuff_; }
		inline auto& bind_func() { return globalctx_.bind_func_; }

		template<class DataT>
		inline void user_data(DataT && data) {
			this->user_data_ = std::forward<DataT>(data);
		}
		template<class DataT>
		inline DataT* user_data() {
			try {
				return std::any_cast<DataT>(&this->user_data_);
			}
			catch (const std::bad_any_cast&) {}
			return nullptr;
		}
		inline void user_data_reset() {
			this->user_data_.reset();
		}
	//protected:
		template<bool iskeepalive = false>
		inline asio::awaitable<error_code> co_start() {
			error_code rec;
			try {
				estate expected = estate::stopped;
				if (!this->state_.compare_exchange_strong(expected, estate::starting))
					asio::detail::throw_error(asio::error::already_started);

				if constexpr (iskeepalive)
					this->keep_alive_options();

				const auto& sptr = this->shared_from_this();
				if (auto rec = co_await this->stream_start(sptr); rec) {
					asio::detail::throw_error(rec);
				}

				if constexpr (proto_able<ProtoType, session_ptr_type>) {
					if (auto rec = co_await this->proto_start(sptr); rec) {
						asio::detail::throw_error(rec);
					}
				}

				expected = estate::starting;
				if (!this->state_.compare_exchange_strong(expected, estate::started))
					asio::detail::throw_error(asio::error::operation_aborted);

				//add session_mgr
				if (bool isadd = this->globalctx_.sessions_.emplace(sptr); !isadd)
					asio::detail::throw_error(asio::error::address_in_use);

				globalctx_.bind_func_->call(event::connect, sptr);

				//if (auto ec = co_await this->recv_t(); ec) {
				//	asio::detail::throw_error(ec);
				//}
				this->transfer_start();
				
				co_return ec_ignore;
			}
			catch (system_error& e) {
				set_last_error(e);
				std:: cout << "svr session co_start:" << e.what() << std::endl;
				//this->stop(e.code());
				rec = e.code();
			}

			if (rec) {
				[[maybe_unused]] auto ret =  co_await this->co_stop(rec);
			}

			co_return rec;
		}

		inline asio::awaitable<error_code> co_stop(const error_code& ec) {
			try {
				estate expected_starting = estate::starting;
				estate expected_started = estate::started;
				if (this->state_.compare_exchange_strong(expected_starting, estate::stopping) ||
					this->state_.compare_exchange_strong(expected_started, estate::stopping)) {
					this->user_data_reset();

					this->transfer_stop();

					auto dptr = this->shared_from_this();
					bool isremove = this->globalctx_.sessions_.erase(dptr);
					if (!isremove) {
						//asio::detail::throw_error(asio::error::operation_aborted);
					}

					if constexpr (proto_able<ProtoType, session_ptr_type>) {
						if (auto rec = co_await this->proto_stop(dptr); rec) {
							asio::detail::throw_error(rec);
						}
					}
		
					if (auto rec = co_await this->stream_stop(dptr); rec) {
						asio::detail::throw_error(rec);
					}

					estate expected_stopping = estate::stopping;
					if (this->state_.compare_exchange_strong(expected_stopping, estate::stopped)) {
						globalctx_.bind_func_->call(event::disconnect, dptr, ec);
					}
					else {
						NET_ASSERT(false);
					}
					co_return ec_ignore;
				}
			}
			catch (system_error& e) {
				set_last_error(e);
				std:: cout << "svr session co_stop:" << e.what() << std::endl;
				co_return e.code();
			}
		}

		inline auto& globalctx() { return globalctx_; }
		inline const auto& state() { return state_; }
	protected:
		nio & cio_;
		
		global_ctx_type& globalctx_;

		std::atomic<estate> state_ = estate::stopped;

		dynamic_buffer<> rbuff_;

		std::any user_data_;
	};
}
