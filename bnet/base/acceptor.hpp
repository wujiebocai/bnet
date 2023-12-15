/*
 * Copyright (c) 2023 bocai
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

namespace bnet::base {
	// default
	template<class ... Args>
	class acceptor {
	public:
		template<class ... FArgs>
		explicit acceptor(FArgs&&... args) {}
	};

    template<class ServerType, is_tcp_by_stream StreamType> 
	class acceptor<ServerType, StreamType> {
	public:
		acceptor(nio& io)
			: server_(static_cast<ServerType&>(*this))
			, cio_(io)
			, acceptor_(this->cio_.context())
			, acceptor_timer_(this->cio_.context())
		{}

		~acceptor() {
			this->acceptor_stop();
		}

        inline asio::awaitable<error_code> acceptor_start(std::string_view host, std::string_view port) {
            try {
				error_code ec, ecignore;
				this->acceptor_.cancel(ecignore);
				this->acceptor_.close(ecignore);

				// parse address and port
				asio::ip::tcp::resolver resolver(this->cio_.context());
				//asio::ip::tcp::endpoint endpoint 
				auto results = resolver.resolve(host, port,
					asio::ip::resolver_base::flags::passive | asio::ip::resolver_base::flags::address_configured, ec);
				if (ec) {
					co_return ec;
				}

				if (results.empty()) {
					co_return asio::error::host_not_found;
				}

				asio::ip::tcp::endpoint endpoint = *results.begin();
				this->acceptor_.open(endpoint.protocol(), ec);
				if (ec) {
					co_return ec;
				}

				this->acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true), ecignore); // set port reuse
				//this->acceptor_->set_option(asio::ip::tcp::no_delay(true));
				//this->acceptor_->non_blocking(true);

				this->acceptor_.bind(endpoint, ec);
				if (ec) {
					co_return ec;
				}

				this->acceptor_.listen(asio::socket_base::max_listen_connections, ec);
				if (ec) {
					co_return ec;
				}

				auto executor = co_await asio::this_coro::executor;
                asio::co_spawn(executor, [this/*, self = this->server_.self_shared_ptr()*/]() { 
					return this->acceptor_start_t(); 
				}, asio::detached);
			}
			catch (system_error & e) {
				//set_last_error(e);
				co_return e.code();
			}

			co_return ec_ignore;
        }

        inline void acceptor_stop() {
            this->acceptor_stop_t();
        }

    protected:
		inline asio::awaitable<void> acceptor_start_t() {
			try {
            	while (this->is_open()) {
					auto session_ptr = this->server_.make_session();
					asio::io_context& ex = session_ptr->cio().context();
            	    auto [ec, sock] = co_await this->acceptor_.async_accept(ex, asio::as_tuple(asio::use_awaitable));
            	    if (!ec) {
						session_ptr->socket().lowest_layer() = std::move(sock);
            	        session_ptr->start();
            	    }
            	    else {
            	        if (ec == asio::error::operation_aborted) {
            	            asio::detail::throw_error(ec);
						    co_return;
					    }

            	        //steady_timer timer(co_await this_coro::executor);
            	        //timer.expires_after(100ms);
						//constexpr asio::steady_timer::duration interval = 100ms;
            	        this->acceptor_timer_.expires_after(std::chrono::milliseconds(100));
            	        co_await this->acceptor_timer_.async_wait(asio::use_awaitable);
            	    }
            	}
			}
			catch (system_error & e) {
				//set_last_error(e);
				this->server_.stop(e.code());
			}
			co_return;
		}

		inline void acceptor_stop_t() {
			error_code ec{};
			this->acceptor_timer_.cancel();
			this->acceptor_.cancel(ec);
			this->acceptor_.close(ec);
		}

		inline bool is_open() const { return this->acceptor_.is_open(); }

		inline std::string listen_address() {
			try {
				return this->acceptor_.local_endpoint().address().to_string();
			}
			catch (system_error & e) { set_last_error(e); }
			return std::string();
		}

		inline unsigned short listen_port() {
			try {
				return this->acceptor_.local_endpoint().port();
			}
			catch (system_error & e) { set_last_error(e); }
			return static_cast<unsigned short>(0);
		}

	protected:
		ServerType & server_;
		nio & cio_;
		asio::ip::tcp::acceptor acceptor_;
		asio::steady_timer acceptor_timer_;
	};

	template<class ServerType, is_udp_by_stream StreamType> 
	class acceptor<ServerType, StreamType> {
	public:
		explicit acceptor(nio& io) 
			: server_(static_cast<ServerType&>(*this))
			, acceptor_(io.context())
			, cio_(io)
		{}

		~acceptor() = default;

		inline asio::awaitable<error_code> acceptor_start(std::string_view host, std::string_view port) {
			try {
				clear_last_error();

				error_code ec, ecignore;

				asio::ip::udp::resolver resolver(this->cio_.context());
				auto results = resolver.resolve(host, port,
					asio::ip::resolver_base::flags::passive | asio::ip::resolver_base::flags::address_configured, ec);
				if (ec) {
					co_return ec;
				}

				if (results.empty()) {
					co_return asio::error::host_not_found;
				}

				asio::ip::udp::endpoint endpoint = *results.begin();

				this->acceptor_.cancel(ecignore);
				this->acceptor_.close(ecignore);
				this->acceptor_.open(endpoint.protocol(), ec);
				if (ec) {
					co_return ec;
				}

				this->acceptor_.set_option(asio::ip::udp::socket::reuse_address(true), ecignore); // set port reuse

				//this->acceptor_.set_option(
				//	asio::ip::multicast::join_group(asio::ip::make_address("ff31::8000:1234")));
				//	asio::ip::multicast::join_group(asio::ip::make_address("239.255.0.1")));


				this->acceptor_.bind(endpoint, ec);
				if (ec) {
					co_return ec;
				}

				auto executor = co_await asio::this_coro::executor;
                asio::co_spawn(executor, this->acceptor_start_t(), asio::detached);
			}
			catch (system_error& e) {
				set_last_error(e);
				co_return e.code();
			}

			co_return ec_ignore;
		}

		inline void acceptor_stop() {
			this->acceptor_stop_t();
		}
	protected:
		inline asio::awaitable<void> acceptor_start_t() { 
			try {
				static_buffer<1024> buffer;
				asio::ip::udp::endpoint remote_endpoint;
				while (this->is_open()) {
					//buffer.reset();
					auto [ec, nrecv] = co_await this->acceptor_.async_receive_from(
						asio::mutable_buffer(buffer.wr_buf(), buffer.wr_size()), 
						remote_endpoint, asio::as_tuple(asio::use_awaitable));

					auto session_ptr = this->server_.get_sessions().find(std::hash<asio::ip::udp::endpoint>()(remote_endpoint));
                    if (ec) {
                        if (ec == asio::error::operation_aborted)
							asio::detail::throw_error(ec);
						
						if (session_ptr /*&& nrecv <= 0*/) {
							session_ptr->stop(ec);
						}
                    }

                    if (nrecv <= 0) continue;

					//buffer.rd_flip(nrecv);

					if (!session_ptr) {
						session_ptr = this->server_.make_session();
						std::cout << "udp acceptor: " << this->server_.session_count() << ", aa:" << std::hash<asio::ip::udp::endpoint>()(remote_endpoint) << std::endl;
						session_ptr->buffer().put(buffer.rd_buf(), nrecv);
						session_ptr->remote_endpoint(remote_endpoint);
						co_await session_ptr->co_start();
					} else {
						error_code sec = session_ptr->handle_recv(ec, std::string_view(reinterpret_cast<
							std::string_view::const_pointer>(buffer.rd_buf()), nrecv));
						if (sec) co_await session_ptr->co_stop(sec);
					}
				}

				asio::detail::throw_error(asio::error::eof);
			}
			catch (system_error& e) {
				set_last_error(e);
				this->server_.stop(e.code());
			}
		}

		inline void acceptor_stop_t() {
			error_code ec{};
			this->acceptor_.shutdown(asio::socket_base::shutdown_both, ec);
			this->acceptor_.close(ec);
		}

		inline bool is_open() const { return this->acceptor_.is_open(); }
	protected:
		ServerType& server_;
		asio::ip::udp::socket acceptor_;
		nio& cio_;
	};
}