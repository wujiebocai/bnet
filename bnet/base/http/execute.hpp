#pragma once

namespace bnet::beast::http {
	struct http_execute {
	protected:
		static inline long constexpr http_execute_timeout   = 15 * 1000;
		template<typename String, typename StrOrInt, class Body, class Fields, class Buffer>
		static asio::awaitable<error_code> execute_trivially_t(
			[[maybe_unused]] asio::io_context& ioc, asio::ip::tcp::resolver& resolver, asio::ip::tcp::socket& socket,
			http::parser<false, Body, typename Fields::allocator_type>& parser,
			Buffer& buffer,
			String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req) {
			// Look up the domain name
			auto [ec, endpoints] = co_await resolver.async_resolve(std::forward<String>(host), tool::to_string(std::forward<StrOrInt>(port)), asio::as_tuple(asio::use_awaitable));
			if (ec) {
                co_return ec;
            }

			// Make the connection on the IP address we get from a lookup
			auto [ec1, endpoint] = co_await asio::async_connect(socket, endpoints, asio::as_tuple(asio::use_awaitable));
            if (ec1) {
				co_return ec;
            }

			auto [ec2, nsent] = co_await http::async_write(socket, req, asio::as_tuple(asio::use_awaitable));
            if (ec2) {
                co_return ec;
            }

			// Then start reading
			auto [ec3, nrecv] = co_await http::async_read(socket, buffer, parser,
                                            asio::as_tuple(asio::use_awaitable));
            if (ec3) {
                co_return ec;
            }

			co_return ec_ignore;
		}

		template<typename String, typename StrOrInt, class Body, class Fields, class Buffer>
		static void execute_impl_t(
			asio::io_context& ioc, asio::ip::tcp::resolver& resolver, asio::ip::tcp::socket& socket,
			http::parser<false, Body, typename Fields::allocator_type>& parser,
			Buffer& buffer,
			String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req) {
			asio::co_spawn(ioc, [&]() -> asio::awaitable<void> { 
				auto ec = co_await http_execute::execute_trivially_t(ioc, resolver, socket, parser, buffer
										, std::forward<String>(host), std::forward<StrOrInt>(port)
										, req);
				if (ec) {
					std::cerr << "http exceute fail. errmsg: " << ec.message() << std::endl;
				}
				co_return;
			}, asio::detached);
		}

	public:
		template<base::is_string_like_or_constructible_c String, typename StrOrInt, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields, class Buffer = beast::flat_buffer>
		http::response<Body, Fields>
		static inline execute(String&& host, StrOrInt&& port,
			http::request<Body, Fields>& req, std::chrono::duration<Rep, Period> timeout) {
			http::parser<false, Body, typename Fields::allocator_type> parser;

			// First assign default value timed_out to last error
			set_last_error(asio::error::timed_out);

			// set default result to unknown
			parser.get().result(http::status::unknown);
			parser.eager(true);

			// The io_context is required for all I/O
			asio::io_context ioc;

			// These objects perform our I/O
			asio::ip::tcp::resolver resolver{ ioc };
			asio::ip::tcp::socket socket{ ioc };

			// This buffer is used for reading and must be persisted
			Buffer buffer;

			// do work
			http_execute::execute_impl_t(ioc, resolver, socket, parser, buffer
				, std::forward<String>(host), std::forward<StrOrInt>(port)
				, req
			);

			// timedout run
			ioc.run_for(timeout);

			error_code ec_ignore{};

			// Gracefully close the socket
			socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec_ignore);
			socket.cancel(ec_ignore);
			socket.close(ec_ignore);

			return parser.release();
		}

		// ----------------------------------------------------------------------------------------

		//template<typename String, typename StrOrInt, class Rep, class Period,
		//	class Body = http::string_body, class Fields = http::fields>
		//typename std::enable_if_t<detail::is_character_string_v<detail::remove_cvref_t<String>>
		//	, http::response<Body, Fields>>
		//static inline execute(String&& host, StrOrInt&& port,
		//	http::request<Body, Fields>& req, std::chrono::duration<Rep, Period> timeout)
		//{
		//	return derived_t::execute(
		//		std::forward<String>(host), std::forward<StrOrInt>(port), req, timeout, std::in_place);
		//}

		template<base::is_string_like_or_constructible_c String, typename StrOrInt, class Body = http::string_body, class Fields = http::fields>
		http::response<Body, Fields>
		static inline execute(String&& host, StrOrInt&& port, http::request<Body, Fields>& req) {
			return http_execute::execute(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				req, std::chrono::milliseconds(http_execute::http_execute_timeout));
		}

		// ----------------------------------------------------------------------------------------

		template<class Rep, class Period, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(web_request& req, std::chrono::duration<Rep, Period> timeout) {
			return http_execute::execute(req.url().host(), req.url().port(), req.base(), timeout);
		}

		template<class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(web_request& req) {
			return http_execute::execute(req, std::chrono::milliseconds(http_execute::http_execute_timeout));
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @brief blocking execute the http request until it is returned on success or failure
		 */
		template<class Rep, class Period, class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(std::string_view url, std::chrono::duration<Rep, Period> timeout) {
			web_request req = http::make_request(url);
			if (get_last_error())
			{
				return http::response<Body, Fields>{ http::status::unknown, 11};
			}
			return http_execute::execute(
				req.host(), req.port(), req.base(), timeout, std::in_place);
		}

		/**
		 * @brief blocking execute the http request until it is returned on success or failure
		 */
		template<class Body = http::string_body, class Fields = http::fields>
		static inline http::response<Body, Fields> execute(std::string_view url) {
			return http_execute::execute(url, std::chrono::milliseconds(http_execute::http_execute_timeout));
		}

		// ----------------------------------------------------------------------------------------

		/**
		 * @brief blocking execute the http request until it is returned on success or failure
		 */
		template<base::is_string_like_or_constructible_c String, typename StrOrInt, class Rep, class Period,
			class Body = http::string_body, class Fields = http::fields>
		http::response<Body, Fields>
		static inline execute(String&& host, StrOrInt&& port,
			std::string_view target, std::chrono::duration<Rep, Period> timeout) {
			web_request req = http::make_request(host, port, target);
			if (get_last_error()) {
				return http::response<Body, Fields>{ http::status::unknown, 11};
			}
			return http_execute::execute(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				req.base(), timeout);
		}

		/**
		 * @brief blocking execute the http request until it is returned on success or failure
		 */
		template<base::is_string_like_or_constructible_c String, typename StrOrInt, class Body = http::string_body, class Fields = http::fields>
		http::response<Body, Fields>
		static inline execute(String&& host, StrOrInt&& port, std::string_view target) {
			return http_execute::execute(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				target, std::chrono::milliseconds(http_execute::http_execute_timeout));
		}
	};
}
