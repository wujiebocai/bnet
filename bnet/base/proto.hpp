/*
 * This file is part of tcpflow by Simson Garfinkel <simsong@acm.org>.
 * Originally by Will Glynn <will@willglynn.com>.
 *
 * This source code is under the GNU Public License (GPL) version 3.
 * See COPYING for details.
 *
 */

#pragma once

#include <filesystem>
#include "tool/str.hpp"
#if defined(NET_USE_HTTP)
#include "base/http/parse.hpp"
#include "base/http/mime_types.hpp"
#include "base/http/multipart.hpp"
#include "base/http/util.hpp"
#include "base/http/flex_body.hpp"
#include "base/http/url.hpp"
#include "base/http/request.hpp"
#include "base/http/response.hpp"
#include "base/http/make.hpp"
#include "base/http/router.hpp"
#include "base/http/execute.hpp"
#endif

namespace bnet::base {
	template<class ... Args>
	class proto {
	public:
		template<class ... FArgs>
		explicit proto([[maybe_unused]] FArgs&&... args) {}
	};

#if defined(NET_USE_HTTP)
	template<class SvrOrCli>
	class http_proto : public http_tag {
	public:
		template<class ... FArgs>
		explicit http_proto([[maybe_unused]] FArgs&&... args) {}

		void fill_route_fail(http::web_request&	req, http::web_response& rep) {
			std::string desc;
			desc.reserve(64);
			desc += "The resource for ";
			desc += req.method_string();
			desc += " \"";
			desc += http::url_decode(req.target());
			desc += "\" was not found";

			rep.fill_page(http::status::not_found, std::move(desc), {}, req.version());
		}

		inline auto&  cli_router() requires is_cli_c<SvrOrCli> { return cli_router_; }
	protected:
		http::http_cli_router cli_router_;
	};
#endif

#if defined(NET_USE_HTTP)
	template<class SvrOrCli>
	class ws_proto : public ws_tag {
	public:
		explicit ws_proto([[maybe_unused]] std::size_t max_buffer_size)
			: buffer_(max_buffer_size) { }

		template<class SessionPtr>
		inline asio::awaitable<error_code> proto_start(SessionPtr&& ptr) {
			this->init(ptr);

			error_code ec;
			if constexpr (is_svr_c<SvrOrCli>) {
				ec = co_await this->upgrade(ptr); //this->recv_upgrade_req_t(ptr);
			}
			else {
				this->ws_control_callback(ptr);
				ec = co_await this->upgrade(ptr, upgrade_rep_);
			}

			ptr->cbfunc()->call(event::upgrade, ptr, ec);

			co_return ec;
		}

		template<class SessionPtr>
		inline asio::awaitable<error_code> proto_stop(SessionPtr&& ptr) {
			try {
				// Set the handshake timeout to a small value, otherwise if the remote don't
				// send a websocket close frame, the async_close's callback will never becalled.
				websocket::stream_base::timeout opt{};
				opt.handshake_timeout = disconnect_timeout_;
				opt.idle_timeout      = websocket::stream_base::none();
				ptr->stream().set_option(opt);

				// Reset the decorator, beacuse the decorator maybe hold the self 
				// shared_ptr, if don't reset it, the session ptr maybe can't detroyed.
				ptr->stream().set_option(
					websocket::stream_base::decorator([](websocket::request_type&) {}));
				ptr->stream().set_option(
					websocket::stream_base::decorator([](websocket::response_type&) {}));
				}
			catch (system_error const& e) {
				set_last_error(e);
			}

			// Can't call close twice
			// TODO return a custom error code
			// BHO_ASSERT(! impl.wr_close);
			if (ptr->stream().is_open()) {
				std::cout << "ws_stream_cp enter async_close" << std::endl;

				// Close the WebSocket connection
				// async_close behavior : 
				// send a websocket close frame to the remote, and wait for recv a websocket close
				// frame from the remote.
				// async_close maybe close the socket directly.
				auto [ec] = co_await ptr->stream().async_close(websocket::close_code::normal, asio::as_tuple(asio::use_awaitable));

				std::cout << "ws_stream_cp leave async_close:" << ec.value() << " " << ec.message() << std::endl;

				// if async close failed, the inner timer of async close will exists for timeout,
				// it will cause the ws client can't be exited, so we reset the timeout to none
				// to notify the timer to exit.
				if (ec) {
					websocket::stream_base::timeout opt{};
					opt.handshake_timeout = websocket::stream_base::none();
					opt.idle_timeout      = websocket::stream_base::none();
					try {
						ptr->stream().set_option(opt);
					}
					catch (system_error const&) {
					}

					co_return ec;
				}
			}

			// If we get here then the connection is closed gracefully
			
			// must Reset the control frame callback. the control frame callback hold the 
			// self shared_ptr, if don't reset it, will cause memory leaks.
			ptr->stream().control_callback();
			co_return ec_ignore;
		}

		inline beast::flat_buffer& ws_buffer() { return buffer_; }
	protected:
		template<class SessionPtr>
		inline void init(SessionPtr&& ptr) {
			websocket::stream_base::timeout opt{};

			if constexpr (is_svr_c<SvrOrCli>) {
				opt = websocket::stream_base::timeout::suggested(beast::role_type::server);
			}
			else {
				opt = websocket::stream_base::timeout::suggested(beast::role_type::client);
			}

			opt.handshake_timeout = connect_timeout_;
			ptr->stream().set_option(opt);
		}

		template<class SessionPtr>
		inline asio::awaitable<error_code> recv_upgrade_req_t(SessionPtr&& ptr) requires is_svr_c<SvrOrCli> {
            // Make the request empty before reading,
			// otherwise the operation behavior is undefined.
            upgrade_req_ = {};
            auto [ec, nrecv] = co_await http::async_read(ptr->socket(), buffer_, upgrade_req_, asio::as_tuple(asio::use_awaitable));
            if (ec) {
                //_handle_upgrade;
                co_return ec;
            }

            this->ws_control_callback(ptr);

            ec = co_await this->upgrade(ptr, upgrade_req_);
            if (ec) {
                co_return ec;
            }

            co_return ec_ignore;
        }

		template<class SessionPtr>
		void ws_control_callback(SessionPtr&& ptr) {
			ptr->stream().control_callback([ptr, this](websocket::frame_type kind, std::string_view payload){
				switch (kind) {
				case websocket::frame_type::ping:
					handle_control_ping(payload, std::move(ptr));
					break;
				case websocket::frame_type::pong:
					handle_control_pong(payload, std::move(ptr));
					break;
				case websocket::frame_type::close:
					handle_control_close(payload, std::move(ptr));
					break;
				default: break;
				}
			});
		}

		template<class SessionPtr>
		inline void handle_control_ping(std::string_view payload, SessionPtr&& ptr) {
			std::ignore = payload;
			std::ignore = ptr;
		}
		template<class SessionPtr>
		inline void handle_control_pong(std::string_view payload, SessionPtr&& ptr) {
			std::ignore = payload;
			std::ignore = ptr;
		}
		template<class SessionPtr>
		inline void handle_control_close(std::string_view payload, SessionPtr&& ptr) {
			std::ignore = payload;
			std::ignore = ptr;
			//derived_t& derive = static_cast<derived_t&>(*this);

			//detail::ignore_unused(payload, this_ptr, ecs);

			//if (derive.state() == state_t::started)
			//{
			//	ASIO2_LOG_DEBUG("ws_stream_cp::_handle_control_close _do_disconnect");

			//	derive._do_disconnect(websocket::error::closed, std::move(this_ptr));
			//}
		}

		template<typename SessionPtr, typename Response>
		inline asio::awaitable<error_code> upgrade(SessionPtr&& ptr, Response& rep) requires is_cli_c<SvrOrCli> {
			auto host = ptr->stream().remote_address();
			// Perform the websocket handshake
			auto [ec] = co_await ptr->stream().async_handshake(rep, host, upgrade_target(), asio::as_tuple(asio::use_awaitable));
			if (ec) {
				co_return ec;
			}

			co_return ec_ignore;
		}

		template<typename SessionPtr, typename Request>
		inline asio::awaitable<error_code> upgrade(SessionPtr&& ptr, Request const& req) requires is_svr_c<SvrOrCli> {
			// Accept the websocket handshake just write response.
			auto [ec] = co_await ptr->stream().async_accept(req, asio::as_tuple(asio::use_awaitable));
			if (ec) {
				co_return ec;
			}

			co_return ec_ignore;
		}

		template<typename SessionPtr>
		inline asio::awaitable<error_code> upgrade(SessionPtr&& ptr) requires is_svr_c<SvrOrCli> {
			// Accept the websocket handshake first read request, then write response.
			auto [ec] = co_await ptr->stream().async_accept(asio::as_tuple(asio::use_awaitable));
			if (ec) {
				co_return ec;
			}

			co_return ec_ignore;
		}

		//inline void handle_upgrade(const error_code& ec) {
			// call event::upgrade
		//}

		inline const std::string& upgrade_target() { return upgrade_target_; }
		inline void upgrade_target(std::string target) { upgrade_target_ = std::move(target); }
	protected:
		std::chrono::steady_clock::duration connect_timeout_ = std::chrono::seconds(30);
		std::chrono::steady_clock::duration disconnect_timeout_ = std::chrono::seconds(30);
		
		beast::flat_buffer buffer_;
        websocket::request_type upgrade_req_;
		websocket::response_type upgrade_rep_;
		std::string upgrade_target_ = "/";
	};
#endif
}
