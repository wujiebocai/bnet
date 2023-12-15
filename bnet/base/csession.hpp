/*
 * Copyright (c) 2023 bocai
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include "asio/experimental/awaitable_operators.hpp"
#include "base/transfer.hpp"

using namespace asio::experimental::awaitable_operators;

namespace bnet::base {
	template<class StreamType, class ProtoType>
	class csession : public StreamType
                   , public ProtoType
				   , public transfer<csession<StreamType, ProtoType>, StreamType, ProtoType, cli_tag>
				   , public std::enable_shared_from_this<csession<StreamType, ProtoType>> {
	public:
		using session_type = csession<StreamType, ProtoType>;
		using session_ptr_type = std::shared_ptr<session_type>;
		using stream_type = StreamType;
        using proto_type = ProtoType;
		using transfer_type = transfer<csession<StreamType, ProtoType>, StreamType, ProtoType, cli_tag>;
        using global_ctx_type = global_ctx<session_type>;
        using socket_type = typename StreamType::socket_type;
		using resolver_type = typename asio::ip::basic_resolver<typename socket_type::protocol_type>;
		using endpoints_type = typename resolver_type::results_type;
		using endpoint_type = typename socket_type::lowest_layer_type::endpoint_type;
		using endpoints_iterator = typename endpoints_type::iterator;
		//using key_type = typename std::conditional<is_udp_socket_v<socket_type>, asio::ip::udp::endpoint, std::size_t>::type;
		using key_type = std::size_t;
	public:
		template<class ...Args>
		explicit csession(global_ctx_type& globalctx, nio& io,
						std::size_t max_buffer_size, Args&&... args)
			: stream_type(std::forward<Args>(args)...)
            , proto_type(max_buffer_size)
			, transfer_type(io, max_buffer_size)
			, cio_(io)
			, globalctx_(globalctx)
		{
		}

		~csession() = default;

        template<bool IsKeepAlive = false, typename Fun> 
		requires is_co_spawn_cb<Fun> || std::same_as<unqualified_t<Fun>, asio::detached_t>
		inline void start(std::string_view host, std::string_view port, Fun&& func) {
			asio::co_spawn(this->cio_.context(), [self = self_shared_ptr(), host, port]() { 
				return self->template co_start<IsKeepAlive>(host, port);
			}, func);
		}

		inline void stop(const error_code& ec = ec_ignore) {
			asio::co_spawn(this->cio_.context(), [self = self_shared_ptr(), ec = std::move(ec)]() { 
				return self->co_stop(ec);
			}, asio::detached);
		}

		template<bool IsKeepAlive = false>
		inline bool reconn() {
			if (!is_stopped()) {
				return false;
			}

			asio::co_spawn(this->cio_.context(), [this, self = self_shared_ptr()]() -> asio::awaitable<void> { 
				auto reconn_interval = (globalctx_.cli_cfg_.reconn_interval > 0 ? globalctx_.cli_cfg_.reconn_interval : 3);
				co_await async_sleep(co_await asio::this_coro::executor, std::chrono::seconds(reconn_interval), asio::use_awaitable);
				co_await this->template co_start<IsKeepAlive>(this->host_, this->port_);
				co_return;
			}, asio::detached);
			
			return true;
		}

		//imp
		inline auto self_shared_ptr() { return this->shared_from_this(); }
		//inline asio::streambuf& buffer() { return buffer_; }
		inline nio& cio() { return cio_; }
		inline auto& bind_func() { return globalctx_.bind_func_; }
		inline dynamic_buffer<>& rbuffer() { return rbuff_; }

		inline bool is_started() const {
			return (this->state_ == estate::started && this->socket().lowest_layer().is_open());
		}
		inline bool is_stopped() const {
			return (this->state_ == estate::stopped && !this->socket().lowest_layer().is_open());
		}
		inline key_type hash_key() const {
			return reinterpret_cast<key_type>(this);
		}

		template<class DataT>
		inline void user_data(DataT&& data) {
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
		//inline auto& host() { return host_; }
	//protected:
        template<bool IsKeepAlive = false>
		inline asio::awaitable<error_code> co_start(std::string_view host, std::string_view port) {
			error_code rec;
			try {
				clear_last_error();

                estate expected = estate::stopped;
			    if (!this->state_.compare_exchange_strong(expected, estate::starting)) {
                    asio::detail::throw_error(asio::error::already_started);
			    }

				asio::steady_timer ctimer(cio_.context());
				auto conn_timeout = (globalctx_.cli_cfg_.conn_timeout > 0 ? globalctx_.cli_cfg_.conn_timeout : 3000);
                ctimer.expires_after(std::chrono::milliseconds(conn_timeout));
                std::variant<error_code, std::monostate> rets = co_await (this->template co_connect<IsKeepAlive>(host, port) || ctimer.async_wait(asio::use_awaitable));
				ctimer.cancel();
                auto idx = rets.index();
                if (idx == 0) {
                    error_code ret = std::get<0>(rets);
                    if (ret) {
                        asio::detail::throw_error(ret);
                    }

					auto dptr = this->shared_from_this();
					if (auto rec = co_await this->stream_start(dptr); rec) {
						asio::detail::throw_error(rec);
					}

					if constexpr (proto_able<ProtoType, session_ptr_type>) {
						if (auto rec = co_await this->proto_start(dptr); rec) {
							asio::detail::throw_error(rec);
						}
					}

					estate expected = estate::starting;
					if (!this->state_.compare_exchange_strong(expected, estate::started)) {
						asio::detail::throw_error(asio::error::operation_aborted);
					}

					if (bool isadd = this->globalctx_.sessions_.emplace(dptr); !isadd) {
						asio::detail::throw_error(asio::error::address_in_use);
                	}
				
					globalctx_.bind_func_->call(event::connect, dptr);

					//if (auto ec = co_await this->recv_t(); ec) { // sync will block
					//	asio::detail::throw_error(ec);
					//}
					this->transfer_start();
					
                    co_return ec_ignore;
                }

				if (idx == 1) {
                    asio::detail::throw_error(asio::error::timed_out);
                }

				if (idx == std::variant_npos) {
                    // variant exception？？
					asio::detail::throw_error(asio::error::operation_aborted);
                }
			}
			catch (system_error& e) {
				set_last_error(e);
				std:: cout << "cli session start_t:" << e.what() << std::endl;
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
                    //set_last_error(ec);
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
						if (ec && globalctx_.cli_cfg_.is_reconn) this->reconn();
					}

                    co_return ec_ignore;
                }
            }
            catch (system_error& e) {
				set_last_error(e);
				std:: cout << "cli session co_stop:" << e.what() << std::endl;

				co_return e.code();
			}
		}

        template<bool IsKeepAlive = false>
        inline asio::awaitable<error_code> co_connect(const std::string_view& host, const std::string_view& port) {
            try {
                this->host_ = host;
			    this->port_ = port;

				resolver_type resolver { cio_.context() };
				auto [ec, endpoints] = co_await resolver.async_resolve(host, port, asio::as_tuple(asio::use_awaitable));
			    if (ec) {
                    co_return ec;
                }

                auto & socket = this->socket().lowest_layer();
                auto handle_func = [&socket, this](endpoint_type item) {
                    error_code ec, ecignore;

                    if (socket.is_open()) {
				        auto oldep = socket.local_endpoint(ecignore);
				        if (ecignore || oldep.protocol() != item.protocol()) {
                            socket.cancel(ecignore);
					        socket.close(ecignore);
                        }
			        }

                    if (!socket.is_open()) {
                        socket.open(item.protocol(), ec);
                        if (ec) {
                            return ec;
                        }

                        // set port reuse
				        socket.set_option(asio::socket_base::reuse_address(true), ecignore);

                        if constexpr (IsKeepAlive)
			    	        this->keep_alive_options();

                        //socket.bind(iter->endpoint());
                    }

                    return ec;
                };
                
                auto dptr = this->shared_from_this();
                for (auto && item : endpoints) {
                    if (auto ec = handle_func(item); ec) {
                        co_return ec;
                    }
                    auto [ec] = co_await dptr->socket().lowest_layer().async_connect(item, asio::as_tuple(asio::use_awaitable));
                    if (ec) {
                        if (ec != asio::error::operation_aborted) continue;
						co_return ec;
                    }
                    else {
                        co_return ec_ignore;
                    }
                }

                co_return asio::error::host_not_found;
            }
			catch (system_error & e) {
				set_last_error(e);
				co_return e.code();
			}
        }

		inline auto& globalctx() { return globalctx_; }
		inline const auto& state() { return state_; }
	protected:
		nio & cio_;

        std::string host_, port_;

		global_ctx_type& globalctx_;
		
		std::atomic<estate> state_ = estate::stopped;

		dynamic_buffer<> rbuff_;

		std::any user_data_;
	};
}