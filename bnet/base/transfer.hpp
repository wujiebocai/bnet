#pragma once

namespace bnet::base {
    template<class ... Args>
	class transfer {};

	template<class DriverType, class StreamType, class ProtoType, class SvrOrCli> 
    requires is_tcp_by_stream<StreamType>
	class transfer<DriverType, StreamType, ProtoType, SvrOrCli> {
    public:
        using transfer_type = transfer<DriverType, StreamType, ProtoType, SvrOrCli>;
	public:
		transfer(std::size_t max_buffer_size)
			: derive_(static_cast<DriverType&>(*this))
			, buffer_(max_buffer_size) {}

		~transfer() = default;

        template<class DataType>
        inline void send(DataType&& data) {
			asio::co_spawn(this->derive_.cio().context(), [this, self = this->derive_.self_shared_ptr(), data = asio::buffer(std::move(data))]() { 
                return this->send_t(std::move(data));
            }, asio::detached);
		}

    protected:
        inline void recv() {
			asio::co_spawn(this->derive_.cio().context(), [this, self = this->derive_.self_shared_ptr()] () -> asio::awaitable<void> { 
                if (auto ec = co_await this->recv_t(); ec) {
                    this->derive_.stop(ec);
                }
                co_return; 
            }, asio::detached);
		}

        //template<class DataType> requires std::is_same_v<DataType, std::string>
        template<class Buffer>
        asio::awaitable<std::size_t> send_t(Buffer&& buffer) {
            error_code rec;
            try {
                if (!this->derive_.is_started())
					asio::detail::throw_error(asio::error::not_connected);
				if (buffer.size() <= 0)
					asio::detail::throw_error(asio::error::invalid_argument);

                //auto [e2, nwritten] = co_await async_write(socket, asio::buffer(data, nread));
                auto [ec, nsent] = co_await asio::async_write(this->derive_.socket(),
                    std::forward<Buffer>(buffer), asio::as_tuple(asio::use_awaitable));
                if (ec) {
                    asio::detail::throw_error(ec);
                }

                co_return nsent;
            }
            catch (system_error& e) {
                //this->derive_.stop(e.code());
                rec = e.code();
            }
            catch (std::exception&) { set_last_error(asio::error::eof); }

            if (rec) {
                [[maybe_unused]] auto ret = co_await this->derive_.stop_t(rec);
            }

            co_return 0;
        }

        asio::awaitable<error_code> recv_t() {
            try {
                while (this->derive_.is_started()) {
                    //auto [ec, nrecv] = co_await this->derive_.stream().async_read_some(this->buffer_);
                    auto [ec, nrecv] = co_await asio::async_read(this->derive_.socket(), this->buffer_, asio::transfer_at_least(1), asio::as_tuple(asio::use_awaitable));
                    if (ec) {
                        co_return ec;
                    }

                    if (nrecv <= 0) continue;

                    this->handle_recv(ec, std::string_view(reinterpret_cast<
							std::string_view::const_pointer>(this->buffer_.data().data()), nrecv));

                    this->buffer_.consume(nrecv);
                }
            }
            catch (system_error& e) {
                co_return e.code();
            }
            co_return ec_ignore;
        }

        inline void handle_recv(error_code ec, const std::string_view& s) {
            std::ignore = ec;
			this->derive_.cbfunc()->call(event::recv, this->derive_.self_shared_ptr(), std::move(s));
		}

    protected:
		DriverType& derive_;

		asio::streambuf buffer_;
    };

    template<class DriverType, class StreamType, class ProtoType, class SvrOrCli> 
    requires is_udp_by_stream<StreamType>
	class transfer<DriverType, StreamType, ProtoType, SvrOrCli> {
    public:
        using transfer_type = transfer<DriverType, StreamType, ProtoType, SvrOrCli>;
	public:
		transfer([[maybe_unused]] std::size_t max_buffer_size)
			: derive_(static_cast<DriverType&>(*this))
			, buffer_() {}

		~transfer() = default;

        template<class DataType>
        inline void send(DataType&& data) {
			asio::co_spawn(this->derive_.cio().context(), [this, self = this->derive_.self_shared_ptr(), data = asio::buffer(std::move(data))]() { 
                return this->send_t(std::move(data));
            }, asio::detached);
		}

        inline auto& remote_endpoint() { return remote_endpoint_; }
        inline void remote_endpoint(const asio::ip::udp::endpoint& endpoint) { this->remote_endpoint_ = std::move(endpoint); }
        inline auto& buffer() { return buffer_; }

        inline void handle_recv(error_code ec, const std::string_view& s) {
            std::ignore = ec;
			this->derive_.cbfunc()->call(event::recv, this->derive_.self_shared_ptr(), std::move(s));
		}
    protected:
        inline void recv() {
			asio::co_spawn(this->derive_.cio().context(), [this, self = this->derive_.self_shared_ptr()]() -> asio::awaitable<void> { 
                if (auto ec = co_await this->recv_t(); ec) {
                    this->derive_.stop(ec);
                }
                co_return; 
            }, asio::detached);
		}

        template<class Buffer>
        asio::awaitable<std::size_t> send_t(Buffer&& buffer) requires is_cli_c<SvrOrCli> {
            error_code rec;
            try {
                if (!this->derive_.is_started())
					asio::detail::throw_error(asio::error::not_connected);
				if (buffer.size() <= 0)
					asio::detail::throw_error(asio::error::invalid_argument);

                auto [ec, nsent] = co_await this->derive_.socket().async_send(std::forward<Buffer>(buffer), asio::as_tuple(asio::use_awaitable));
                if (ec) {
                    asio::detail::throw_error(ec);
                }

                co_return nsent;
            }
            catch (system_error& e) {
                //this->derive_.stop(e.code());
                rec = e.code();
            }
            catch (std::exception&) { set_last_error(asio::error::eof); }

            if (rec) {
                [[maybe_unused]] auto ret = co_await this->derive_.stop_t(rec);
            }

            co_return 0;
        }

        template<class Buffer>
        asio::awaitable<std::size_t> send_t(Buffer&& buffer) requires is_svr_c<SvrOrCli> {
            error_code rec;
            try {
                if (!this->derive_.is_started())
					asio::detail::throw_error(asio::error::not_connected);
				if (buffer.size() <= 0)
					asio::detail::throw_error(asio::error::invalid_argument);

                auto [ec, nsent] = co_await this->derive_.socket().async_send_to(std::forward<Buffer>(buffer), 
                                        this->remote_endpoint_, asio::as_tuple(asio::use_awaitable));
                if (ec) {
                    asio::detail::throw_error(ec);
                }

                co_return nsent;
            }
            catch (system_error& e) {
                //this->derive_.stop(e.code());
                rec = e.code();
            }
            catch (std::exception&) { set_last_error(asio::error::eof); }

            if (rec) {
                [[maybe_unused]] auto ret = co_await this->derive_.stop_t(rec);
            }

            co_return 0;
        }

        template<class Buffer>
        asio::awaitable<std::size_t> send_t(std::string_view host, std::string_view port, Buffer&& buffer) {
            //using resolver_type = typename asio::ip::udp::resolver;
			//using endpoints_type = typename resolver_type::results_type;

            error_code rec;
            try {
                if (!this->derive_.is_started())
					asio::detail::throw_error(asio::error::not_connected);
				if (buffer.size() <= 0)
					asio::detail::throw_error(asio::error::invalid_argument);

                asio::ip::udp::resolver resolver(this->cio_.context());
                auto [ec, endpoints] = co_await resolver.async_resolve(host, port, asio::as_tuple(asio::use_awaitable));
			    if (ec) {
                    co_return ec;
                }

                for (auto& iter = endpoints.begin(); iter != endpoints.end(); ++iter) {
                    auto [ec, nsent] = co_await this->derive_.socket().async_send_to(std::forward<Buffer>(buffer), 
                                        this->remote_endpoint_, asio::as_tuple(asio::use_awaitable));
                    if (!ec) {
                        co_return nsent;
                    }
                }

                co_return 0;
            }
            catch (system_error& e) {
                //this->derive_.stop(e.code());
                rec = e.code();
            }
            catch (std::exception&) { set_last_error(asio::error::eof); }

            if (rec) {
                [[maybe_unused]] auto ret = co_await this->derive_.stop_t(rec);
            }

            co_return 0;
        }
        
		inline asio::awaitable<error_code> recv_t() requires is_cli_c<SvrOrCli> {
            try {
                while (this->derive_.is_started()) {
                    //this->buffer_.wr_reserve(init_buffer_size_);
                    //this->buffer_.reset();
                    auto [ec, nrecv] = co_await this->derive_.socket().async_receive(asio::mutable_buffer(this->buffer_.wr_buf(), this->buffer_.wr_size()), 
                                            asio::as_tuple(asio::use_awaitable));
                    if (ec) {
                        co_return ec;
                    }

                    if (nrecv <= 0) continue;

                    //this->buffer_.wr_flip(nrecv);

					this->handle_recv(ec, std::string_view(reinterpret_cast<
						std::string_view::const_pointer>(this->buffer_.rd_buf()), nrecv));
                }
            }
            catch (system_error& e) {
                co_return e.code();
            }

            co_return ec_ignore;
		}

        inline asio::awaitable<error_code> recv_t() requires is_svr_c<SvrOrCli> {
            auto blen = this->buffer_.rd_size();
            if (blen > 0) {
                this->handle_recv(ec_ignore, std::string_view(reinterpret_cast<
					std::string_view::const_pointer>(this->buffer_.rd_buf()), blen));

                this->buffer_.reset();
            }

            co_return ec_ignore;
        }

    protected:
        DriverType& derive_;

        asio::ip::udp::endpoint remote_endpoint_;
        
        dynamic_buffer<> buffer_;
		const std::size_t init_buffer_size_ = 1024;
    };

#if defined(NET_USE_HTTP)
    template<class DriverType, class StreamType, class ProtoType, class SvrOrCli> 
    requires is_http_proto<ProtoType> && is_tcp_by_stream<StreamType>
	class transfer<DriverType, StreamType, ProtoType, SvrOrCli> {
    public:
        using transfer_type = transfer<DriverType, StreamType, ProtoType, SvrOrCli>;
	public:
		transfer([[maybe_unused]] std::size_t max_buffer_size)
			: derive_(static_cast<DriverType&>(*this))
            , rbuff_(max_buffer_size) { }

		~transfer() = default;

        template<class DataType>
        inline void send(DataType&& data) {
            if constexpr (is_c_str_c<DataType>) {
                asio::co_spawn(this->derive_.cio().context(), [this, self = this->derive_.self_shared_ptr(), data = std::string(std::move(data))]() { 
                    return this->send_t(data);
                }, asio::detached);
            }
            else {
                asio::co_spawn(this->derive_.cio().context(), [this, self = this->derive_.self_shared_ptr(), data = std::move(data)]() { 
                    return this->send_t(data);
                }, asio::detached);
            }
		}

        template<class DataType, class HandleFunc>
        inline void send(DataType&& data, HandleFunc&& f) {
            if constexpr (is_c_str_c<DataType>) {
                asio::co_spawn(this->derive_.cio().context(), [this, self = this->derive_.self_shared_ptr(), data = std::string(std::move(data)), f = std::move(f)]() { 
                    this->derive_.cli_router().handle_func(std::move(f));
                    return this->send_t(data);
                }, asio::detached);
            }
            else {
                asio::co_spawn(this->derive_.cio().context(), [this, self = this->derive_.self_shared_ptr(), data = std::move(data), f = std::move(f)]() { 
                    this->derive_.cli_router().handle_func(std::move(f));
                    return this->send_t(data);
                }, asio::detached);
            }
		}

    protected:
        inline void recv() {
			asio::co_spawn(this->derive_.cio().context(), [this, self = this->derive_.self_shared_ptr()]() -> asio::awaitable<void> { 
                if (auto ec = co_await this->recv_t(); ec) {
                    this->derive_.stop(ec);
                }
                co_return; 
            }, asio::detached);
		}

        template<bool IsRequest, class Body, class Fields>
		inline void check_http_message_t(const http::message<IsRequest, Body, Fields>& msg) {
			// https://datatracker.ietf.org/doc/html/rfc2616#section-14.13
			// If an http message header don't has neither "Content-Length" nor "Transfer-Encoding"(chunk)
			// Then the receiver may not be able to parse the http message normally.
			if (!msg.chunked()) {
				if (msg.find(http::field::content_length) == msg.end()) {
					http::try_prepare_payload(const_cast<http::message<IsRequest, Body, Fields>&>(msg));
				}
			}
		}

        template<is_string_like_or_constructible_c Data>
        asio::awaitable<std::size_t> send_t(Data&& data) {
            error_code rec;
            try {
                if (!this->derive_.is_started())
					asio::detail::throw_error(asio::error::not_connected);
				//if (data.size() <= 0)
				//	asio::detail::throw_error(asio::error::invalid_argument);

                //auto [e2, nwritten] = co_await async_write(socket, asio::buffer(data, nread));
                auto [ec, nsent] = co_await asio::async_write(this->derive_.socket(),
                    asio::buffer(std::move(data)), asio::as_tuple(asio::use_awaitable));
                if (ec) {
                    asio::detail::throw_error(ec);
                }

                co_return nsent;
            }
            catch (system_error& e) {
                //this->derive_.stop(e.code());
                rec = e.code();
            }
            catch (std::exception&) { set_last_error(asio::error::eof); }

            if (rec) {
                [[maybe_unused]] auto ret = co_await this->derive_.stop_t(rec);
            }

            co_return 0;
        }

        template<bool IsRequest, class Body, class Fields>
		inline asio::awaitable<std::size_t> send_t(const http::message<IsRequest, Body, Fields>& data) {
            error_code rec;
            try {
                if (!this->derive_.is_started())
					asio::detail::throw_error(asio::error::not_connected);

                check_http_message_t(data);

                auto [ec, nsent] = co_await http::async_write(this->derive_.socket(), data, asio::as_tuple(asio::use_awaitable));
                if (ec) {
                    asio::detail::throw_error(ec);
                }

                co_return nsent;
            }
            catch (system_error& e) {
                rec = e.code();
            }
            catch (std::exception&) { set_last_error(asio::error::eof); }

            if (rec) {
                [[maybe_unused]] auto ret = co_await this->derive_.stop_t(rec);
            }

            co_return 0;
		}

		template<class Body, class Fields>
		inline asio::awaitable<std::size_t> send_t(const http::http_request_impl_t<Body, Fields>& data) {
            error_code rec;
            try {
                if (!this->derive_.is_started())
					asio::detail::throw_error(asio::error::not_connected);

                check_http_message_t(data.base());

                auto [ec, nsent] = co_await http::async_write(this->derive_.socket(), data.base(), asio::as_tuple(asio::use_awaitable));
                if (ec) {
                    asio::detail::throw_error(ec);
                }

                co_return nsent;
            }
            catch (system_error& e) {
                rec = e.code();
            }
            catch (std::exception&) { set_last_error(asio::error::eof); }

            if (rec) {
                [[maybe_unused]] auto ret = co_await this->derive_.stop_t(rec);
            }

            co_return 0;
		}

		template<class Body, class Fields>
		inline asio::awaitable<std::size_t> send_t(const http::http_response_impl_t<Body, Fields>& data) {
            error_code rec;
            try {
                if (!this->derive_.is_started())
					asio::detail::throw_error(asio::error::not_connected);

                check_http_message_t(data.base());

                auto [ec, nsent] = co_await http::async_write(this->derive_.socket(), data, asio::as_tuple(asio::use_awaitable));
                if (ec) {
                    asio::detail::throw_error(ec);
                }

                co_return nsent;
            }
            catch (system_error& e) {
                rec = e.code();
            }
            catch (std::exception&) { set_last_error(asio::error::eof); }

            if (rec) {
                [[maybe_unused]] auto ret = co_await this->derive_.stop_t(rec);
            }

            co_return 0;
		}

        inline asio::awaitable<error_code> recv_t() requires is_svr_c<SvrOrCli> {
            try {
                set_rsp_refresh();

                while (this->derive_.is_started()) {
                    // Make the request empty before reading,
				    // otherwise the operation behavior is undefined.
				    this->req_.reset();
                    auto [ec, nrecv] = co_await http::async_read(this->derive_.socket(), rbuff_, req_,
                                            asio::as_tuple(asio::use_awaitable));
                    if (ec) {
                        co_return ec;
                    }

                    std::string_view target = this->req_.target();
				    http::http_parser_parse_url(
					    target.data(), target.size(), 0, std::addressof(req_.get_url().parser()));
                        
                    rep_.result(http::status::unknown);
				    rep_.keep_alive(req_.keep_alive());
                    
                    co_await this->handle_recv_s();
                    //auto ret = co_await this->handle_recv_s();
                    //if (ret == 0) {
                    //    std::cout << "http handle request fail:" << req_.target() << std::endl;
                    //}
                }
            }
            catch (system_error& e) {
                //this->derive_.stop(e.code());
                co_return e.code();
            }

            co_return ec_ignore;
		}

        inline asio::awaitable<error_code> recv_t() requires is_cli_c<SvrOrCli> {
            try {
                while (this->derive_.is_started()) {
                    // Make the request empty before reading,
				    // otherwise the operation behavior is undefined.
				    this->rep_.reset();

                    auto [ec, nrecv] = co_await http::async_read(this->derive_.socket(), rbuff_, rep_,
                                            asio::as_tuple(asio::use_awaitable));
                    
                    // 路由
                    this->handle_recv_c(ec);

                    if (ec) {
                        co_return ec;
                    }
                }
            }
            catch (system_error& e) {
                //this->derive_.stop(e.code());
                co_return e.code();
            }

            co_return ec_ignore;
		}

        inline asio::awaitable<void> handle_recv_s() {
            std::string path(this->req_.path());
            auto method = this->req_.method();
            auto ret = this->derive_.globalval().handle(method, path);

            if (ret && ret->handlers_) {
                ret->handlers_(req_, rep_);
            }
            else {
                this->derive_.fill_route_fail(req_, rep_);
                co_await this->send_t(rep_);
            }
            co_return;
		}

        inline void handle_recv_c(const error_code& ec) {
            //this->derive_.cbfunc()->call(event::recv, this->derive_.self_shared_ptr(), req_, rep_);
            //auto func = this->derive_.cli_router().handle();
            //if (func) {
            //    func(req_, rep_);
            //}
            this->derive_.cli_router().handle(ec, rep_);
        }

        inline void set_rsp_refresh() {
            std::weak_ptr<DriverType> wptr = this->derive_.self_shared_ptr();
            rep_.refresh([wptr] (const http::web_response& rep) mutable {
                auto sptr = wptr.lock();
                if (sptr) {
                    sptr->send(rep);
                }
            });
        }

    protected:
        DriverType& derive_;
        beast::flat_buffer rbuff_;

        http::web_request	req_;
		http::web_response	rep_;
    };
#endif

#if defined(NET_USE_HTTP)
    template<class DriverType, class StreamType, class ProtoType, class SvrOrCli> 
    requires is_ws_proto<ProtoType> && is_tcp_by_stream<StreamType>
	class transfer<DriverType, StreamType, ProtoType, SvrOrCli> {
    public:
        using transfer_type = transfer<DriverType, StreamType, ProtoType, SvrOrCli>;
	public:
		transfer([[maybe_unused]] std::size_t max_buffer_size)
			: derive_(static_cast<DriverType&>(*this)) {}

		~transfer() = default;

        template<class DataType>
        inline void send(DataType&& data) {
			if constexpr (is_string_like_or_constructible<unqualified_t<DataType>>::value) {
                asio::co_spawn(this->derive_.cio().context(), [this, self = this->derive_.self_shared_ptr(), data = asio::buffer(std::move(data))]() { 
                    return this->send_t(std::move(data));
                }, asio::detached);
            } else {
                asio::co_spawn(this->derive_.cio().context(), [this, self = this->derive_.self_shared_ptr(), data = std::move(data)]() { 
                    return this->send_t(std::move(data));
                }, asio::detached);
            }
		}
    protected:
        inline void recv() {
			asio::co_spawn(this->derive_.cio().context(), [this, self = this->derive_.self_shared_ptr()]() -> asio::awaitable<void> { 
                if (auto ec = co_await this->recv_t(); ec) {
                    this->derive_.stop(ec);
                }
                co_return; 
            }, asio::detached);
		}
        
        template<class Buffer>
        asio::awaitable<std::size_t> send_t(Buffer&& data) {
            error_code rec;
            try {
                if (!this->derive_.is_started())
					asio::detail::throw_error(asio::error::not_connected);

                auto [ec, nsent] = co_await this->derive_.stream().async_write(std::forward<Buffer>(data), asio::as_tuple(asio::use_awaitable));
                if (ec) {
                    asio::detail::throw_error(ec);
                }

                co_return nsent;
            }
            catch (system_error& e) {
                //this->derive_.stop(e.code());
                set_last_error(e.code());
                rec = e.code();
            }
            catch (std::exception&) { set_last_error(asio::error::eof); }

            if (rec) {
                [[maybe_unused]] auto ret = co_await this->derive_.stop_t(rec);
            }

            co_return 0;
        }

        template<bool IsRequest, class Body, class Fields>
        asio::awaitable<std::size_t> send_t(http::message<IsRequest, Body, Fields>& data) {
            error_code rec;
            try {
                if (!this->derive_.is_started())
					asio::detail::throw_error(asio::error::not_connected);
				
                std::ostringstream oss;
			    oss << data;

                auto [ec, nsent] = co_await this->derive_.stream().async_write(asio::buffer(oss.str()), asio::as_tuple(asio::use_awaitable));
                if (ec) {
                    asio::detail::throw_error(ec);
                }

                co_return nsent;
            }
            catch (system_error& e) {
                //this->derive_.stop(e.code());
                set_last_error(e.code());
                rec = e.code();
            }
            catch (std::exception&) { set_last_error(asio::error::eof); }

            if (rec) {
                [[maybe_unused]] auto ret = co_await this->derive_.stop_t(rec);
            }

            co_return 0;
        }

		template<class Body, class Fields>
		inline asio::awaitable<std::size_t> send_t(http::http_request_impl_t<Body, Fields>& data) {
			co_return this->send_t(data.base());
		}

		template<class Body, class Fields>
		inline asio::awaitable<std::size_t> send_t(http::http_response_impl_t<Body, Fields>& data) {
			co_return this->send_t(data.base());
		}

        inline asio::awaitable<error_code> recv_t() {
            try {
                auto& buffer = this->derive_.ws_buffer();
                while (this->derive_.is_started()) {
                    auto [ec, nrecv] = co_await this->derive_.stream().async_read(buffer, asio::as_tuple(asio::use_awaitable));
                    if (ec) {
                        co_return ec;
                    }

                    // bytes_recvd : The number of bytes in the streambuf's get area up to and including the delimiter.
                    if (nrecv <= 0) continue;

                    this->handle_recv(ec, std::string_view(reinterpret_cast<
							std::string_view::const_pointer>(buffer.data().data()), nrecv));

                    buffer.consume(nrecv);
                }
            }
            catch (system_error& e) {
                //this->derive_.stop(e.code());
                co_return e.code();
            }

            co_return ec_ignore;
		}

        inline void handle_recv(error_code ec, const std::string_view& s) {
            std::ignore = ec;
			this->derive_.cbfunc()->call(event::recv, this->derive_.self_shared_ptr(), std::move(s));
		}

        //inline websocket::request_type& upgrade_req() { return upgrade_req_; }
    protected:
        DriverType& derive_;
    };
#endif

    template<class DriverType, class StreamType, class ProtoType, class SvrOrCli> 
    requires is_kcp_stream<StreamType> && is_udp_by_stream<StreamType>
	class transfer<DriverType, StreamType, ProtoType, SvrOrCli> {
    public:
        using transfer_type = transfer<DriverType, StreamType, ProtoType, SvrOrCli>;
	public:
		transfer([[maybe_unused]] std::size_t max_buffer_size)
			: derive_(static_cast<DriverType&>(*this))
			, buffer_() {}

		~transfer() = default;

        template<class DataType>
        inline void send(DataType&& data) {
			asio::co_spawn(this->derive_.cio().context(), [this, self = this->derive_.self_shared_ptr(), data = asio::buffer(std::move(data))]() { 
                return this->send_t(std::move(data));
            }, asio::detached);
		}

        inline auto& buffer() { return buffer_; }

        inline error_code handle_recv(error_code ec, std::string_view s) {
            if (ec) {
                return ec;
            }
			//this->derive_.cbfunc()->call(event::recv, this->derive_.self_shared_ptr(), std::move(s));
            auto ptr = this->derive_.self_shared_ptr();
            auto kec = this->derive_.kcp_handle_recv_t(ptr, s);
            if (kec) {
                return kec;
            }

            return ec_ignore;
		}
    protected:
        inline void recv() {
			asio::co_spawn(this->derive_.cio().context(), [this, self = this->derive_.self_shared_ptr()]() -> asio::awaitable<void> { 
                if (auto ec = co_await this->recv_t(); ec) {
                    this->derive_.stop(ec);
                }
                co_return; 
            }, asio::detached);
		}
/*
        template<class Buffer>
		inline asio::awaitable<std::size_t> send_t(Buffer&& buffer) requires is_cli_c<SvrOrCli> {
			auto buffer = asio::buffer(std::move(data));

			int ret = kcp::ikcp_send(this->derive_.kcp(), (const char *)buffer.data(), (int)buffer.size());

			switch (ret) {
			case  0: set_last_error(error_code{}                        ); break;
			case -1: set_last_error(asio::error::invalid_argument       ); break;
			case -2: set_last_error(asio::error::no_memory              ); break;
			default: set_last_error(asio::error::operation_not_supported); break;
			}

			if (ret == 0) {
				kcp::ikcp_flush(this->derive_.kcp());
			}

			return std::size_t(ret);
		}
*/
        template<class Buffer>
        asio::awaitable<std::size_t> send_t(Buffer&& buffer) {
            error_code rec;
            try {
                if (!this->derive_.is_started())
					asio::detail::throw_error(asio::error::not_connected);
				if (buffer.size() <= 0)
					asio::detail::throw_error(asio::error::invalid_argument);

                int ret = kcp::ikcp_send(this->derive_.kcp(), (const char *)buffer.data(), (int)buffer.size());

			    switch (ret) {
			    case  0: set_last_error(error_code{}                        ); break;
			    case -1: set_last_error(asio::error::invalid_argument       ); break;
			    case -2: set_last_error(asio::error::no_memory              ); break;
			    default: set_last_error(asio::error::operation_not_supported); break;
			    }

			    if (ret == 0) {
			    	kcp::ikcp_flush(this->derive_.kcp());
			    }

                co_return std::size_t(ret);
            }
            catch (system_error& e) {
                //this->derive_.stop(e.code());
                rec = e.code();
            }
            catch (std::exception&) { set_last_error(asio::error::eof); }

            if (rec) {
                [[maybe_unused]] auto ret = co_await this->derive_.stop_t(rec);
            }

            co_return 0;
        }
        
		inline asio::awaitable<error_code> recv_t() requires is_cli_c<SvrOrCli> {
            try {
                while (this->derive_.is_started()) {
                    //this->buffer_.wr_reserve(init_buffer_size_);
                    //this->buffer_.reset();
                    auto [ec, nrecv] = co_await this->derive_.socket().async_receive(asio::mutable_buffer(this->buffer_.wr_buf(), this->buffer_.wr_size()), 
                                            asio::as_tuple(asio::use_awaitable));
                    if (ec) {
                        co_return ec;
                    }

                    if (nrecv <= 0) continue;

                    //this->buffer_.wr_flip(nrecv);

					//this->handle_recv(ec, std::string_view(reinterpret_cast<
					//	std::string_view::const_pointer>(this->buffer_.rd_buf()), nrecv));
                    ec = this->handle_recv(ec, std::string_view(reinterpret_cast<
						std::string_view::const_pointer>(this->buffer_.rd_buf()), nrecv));
                    if (ec) {
                        co_return ec;
                    }
                }
            }
            catch (system_error& e) {
                co_return e.code();
            }

            co_return ec_ignore;
		}

        inline asio::awaitable<error_code> recv_t() requires is_svr_c<SvrOrCli> {
            //auto ptr = this->derive_.self_shared_ptr();
            //auto ec = this->derive_.kcp_handle_recv_t(ptr, ec);
            //if (ec) {
            //    co_return ec;
            //}

            auto ec = this->handle_recv(ec_ignore, std::string_view(reinterpret_cast<
				std::string_view::const_pointer>(this->buffer_.rd_buf()), this->buffer_.rd_size()));

            this->buffer_.reset();

            co_return ec;
        }

    protected:
        DriverType& derive_;
        
        dynamic_buffer<> buffer_;
		const std::size_t init_buffer_size_ = 1024;
    };
}