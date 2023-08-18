#pragma once

//#include <memory>
#include "base/socket.hpp"
#include "kcp_util.hpp"
//#include "base/iopool.hpp"
//#include "base/error.hpp"
//#include "base/timer.hpp"
//#include "opt/kcp/kcp_util.hpp"

namespace bnet::base {
	template<class ... Args>
	class stream_ctx {
	public:
		template<class ... FArgs>
		explicit stream_ctx(FArgs&&...) {}
	};

	//template<class ... Args>
	//class stream {};
///////////////////////////binary stream//////////////////////////////////////////////////////////////////////////
	template<class SocketType>
	class binary_stream : public SocketType {
    public:
        using socket_type = typename SocketType::socket_type;
        using stream_type = void;
	public:
		template<class ...Args>
		explicit binary_stream(Args&&... args)
			: SocketType(std::forward<Args>(args)...)
		{
		}

		~binary_stream() = default;
	protected:
		template<class SessionPtr>
		inline asio::awaitable<error_code> stream_start([[maybe_unused]] SessionPtr&& ptr) {
			//std::ignore = ptr;
			co_return ec_ignore;
		}

		template<class SessionPtr>
		inline asio::awaitable<error_code> stream_stop([[maybe_unused]] SessionPtr&& ptr) {
			if constexpr (is_tcp_by_socket<socket_type>) {
				SocketType::close();
			}

			//if (is_udp_by_socket<socket_type> && is_cli_c<SvrOrCli>) {
			//	SocketType::close();
			//}
			
			co_return ec_ignore;
		}
	};
///////////////////ssl stream///////////////////////////////////////////////////////////////////
#if defined(NET_USE_SSL)
	template<class SocketType>
	class ssl_stream
		: public SocketType {
	public:
		using stream_type = typename SocketType::socket_type;
		using socket_type = typename stream_type::next_layer_type;
		using handshake_type = typename asio::ssl::stream_base::handshake_type;

	public:
		template<class ...Args>
		explicit ssl_stream(nio& io, handshake_type type, Args&&... args)
			: SocketType(std::forward<Args>(args)...)
			, ssl_type_(type)
			//, ssl_io_(io)
		{
		}

		~ssl_stream() = default;
		
	protected:
		template<class SessionPtr>
		inline asio::awaitable<error_code> stream_start(SessionPtr&& ptr) {
			auto [ec] = co_await this->socket().async_handshake(ssl_type_, asio::as_tuple(asio::use_awaitable));
			if (ec) {
				co_return ec;
			}

			ptr->cbfunc()->call(event::handshake, ptr, ec_ignore);

			co_return ec_ignore;
		}

		template<class SessionPtr>
		inline asio::awaitable<error_code> stream_stop([[maybe_unused]] SessionPtr&& ptr) {
			//std::ignore = ptr;
			error_code ecignore{};
			this->socket().lowest_layer().cancel(ecignore);

			auto [ec] = co_await this->socket().async_shutdown(asio::as_tuple(asio::use_awaitable));
			if (ec) {
				co_return ec;
			}

			//SocketType::close();
			this->socket().lowest_layer().close();

			//SSL_clear(this->socket().native_handle());

			co_return ec_ignore;
		}

	protected:
		//nio& ssl_io_;
		//asio::steady_timer ssl_timer_;
		handshake_type ssl_type_;
	};
	// ssl
	template<is_ssl_stream StreamType>
	class stream_ctx<StreamType> : public asio::ssl::context {
	public:
		explicit stream_ctx(svr_tag, asio::ssl::context::method method = asio::ssl::context::sslv23)
			: asio::ssl::context(method)
		{
			// default_workarounds : SSL_OP_ALL
			//	All of the above bug workarounds.
			//	It is usually safe to use SSL_OP_ALL to enable the bug workaround options if
			//  compatibility with somewhat broken implementations is desired.

			// single_dh_use : SSL_OP_SINGLE_DH_USE
			//	Always create a new key when using temporary / ephemeral
			//  DH parameters(see ssl_ctx_set_tmp_dh_callback(3)).
			//  This option must be used to prevent small subgroup attacks,
			//  when the DH parameters were not generated using "strong" 
			//  primes(e.g.when using DSA - parameters, see dhparam(1)).
			//  If "strong" primes were used, it is not strictly necessary
			//  to generate a new DH key during each handshake but it is 
			//  also recommended.
			//  SSL_OP_SINGLE_DH_USE should therefore be enabled whenever
			//  temporary / ephemeral DH parameters are used.
			this->set_options(
				asio::ssl::context::default_workarounds |
				asio::ssl::context::no_sslv2 |
				asio::ssl::context::no_sslv3 |
				asio::ssl::context::single_dh_use
			);
		}

		explicit stream_ctx(cli_tag, asio::ssl::context::method method = asio::ssl::context::sslv23)
			: asio::ssl::context(method)
		{
		}

		~stream_ctx() {
		}

		/**
		 *
		 * >> openssl create your certificates and sign them
		 * ------------------------------------------------------------------------------------------------
		 * // 1. Generate Server private key
		 * openssl genrsa -des3 -out server.key 1024
		 * // 2. Generate Server Certificate Signing Request(CSR)
		 * openssl req -new -key server.key -out server.csr -config openssl.cnf
		 * // 3. Generate Client private key
		 * openssl genrsa -des3 -out client.key 1024
		 * // 4. Generate Client Certificate Signing Request(CSR)
		 * openssl req -new -key client.key -out client.csr -config openssl.cnf
		 * // 5. Generate CA private key
		 * openssl genrsa -des3 -out ca.key 2048
		 * // 6. Generate CA Certificate file
		 * openssl req -new -x509 -key ca.key -out ca.crt -days 3650 -config openssl.cnf
		 * // 7. Generate Server Certificate file
		 * openssl ca -in server.csr -out server.crt -cert ca.crt -keyfile ca.key -config openssl.cnf
		 * // 8. Generate Client Certificate file
		 * openssl ca -in client.csr -out client.crt -cert ca.crt -keyfile ca.key -config openssl.cnf
		 * // 9. Generate dhparam file
		 * openssl dhparam -out dh1024.pem 1024
		 *
		 */

		 /**
		  * server set_verify_mode :
		  *   "verify_peer", ca_cert_buffer can be empty.
		  *      Whether the client has a certificate or not is ok.
		  *   "verify_fail_if_no_peer_cert", ca_cert_buffer can be empty.
		  *      Whether the client has a certificate or not is ok.
		  *   "verify_peer | verify_fail_if_no_peer_cert", ca_cert_buffer cannot be empty.
		  *      Client must use certificate, otherwise handshake will be failed.
		  * client set_verify_mode :
		  *   "verify_peer", ca_cert_buffer cannot be empty.
		  *   "verify_none", ca_cert_buffer can be empty.
		  * private_cert_buffer,private_key_buffer,private_password always cannot be empty.
		  */
		inline error_code set_cert_buffer(
			std::string_view ca_cert_buffer,
			std::string_view private_cert_buffer,
			std::string_view private_key_buffer,
			std::string_view private_password
		) noexcept {
			error_code ec{};
			do {
				this->set_password_callback([password = std::string{ private_password }]
				(std::size_t max_length, asio::ssl::context_base::password_purpose purpose)->std::string {
					ignore_unused(max_length, purpose);

					return password;
				}, ec);

				if (ec)
					break;

				NET_ASSERT(!private_cert_buffer.empty() && !private_key_buffer.empty());

				this->use_certificate(asio::buffer(private_cert_buffer), asio::ssl::context::pem, ec);
				if (ec)
					break;

				this->use_private_key(asio::buffer(private_key_buffer), asio::ssl::context::pem, ec);
				if (ec)
					break;

				if (!ca_cert_buffer.empty())
				{
					this->add_certificate_authority(asio::buffer(ca_cert_buffer), ec);
					if (ec)
						break;
				}
			} while (false);

			set_last_error(ec);

			return ec;
		}

		/**
		 * server set_verify_mode :
		 *   "verify_peer", ca_cert_buffer can be empty.
		 *      Whether the client has a certificate or not is ok.
		 *   "verify_fail_if_no_peer_cert", ca_cert_buffer can be empty.
		 *      Whether the client has a certificate or not is ok.
		 *   "verify_peer | verify_fail_if_no_peer_cert", ca_cert_buffer cannot be empty.
		 *      Client must use certificate, otherwise handshake will be failed.
		 * client set_verify_mode :
		 *   "verify_peer", ca_cert_buffer cannot be empty.
		 *   "verify_none", ca_cert_buffer can be empty.
		 * private_cert_buffer,private_key_buffer,private_password always cannot be empty.
		 */
		inline error_code set_cert_file(
			const std::string& ca_cert_file,
			const std::string& private_cert_file,
			const std::string& private_key_file,
			const std::string& private_password
		) noexcept {
			error_code ec{};
			do {
				this->set_password_callback([password = private_password]
				(std::size_t max_length, asio::ssl::context_base::password_purpose purpose)->std::string {
					ignore_unused(max_length, purpose);

					return password;
				}, ec);
				if (ec)
					break;

				NET_ASSERT(!private_cert_file.empty() && !private_key_file.empty());

				this->use_certificate_chain_file(private_cert_file, ec);
				if (ec)
					break;

				this->use_private_key_file(private_key_file, asio::ssl::context::pem, ec);
				if (ec)
					break;

				if (!ca_cert_file.empty())
				{
					this->load_verify_file(ca_cert_file, ec);
					if (ec)
						break;
				}
			} while (false);

			set_last_error(ec);

			return ec;
		}

		inline error_code set_dh_buffer(std::string_view dh_buffer) noexcept {
			error_code ec{};

			if (!dh_buffer.empty())
				this->use_tmp_dh(asio::buffer(dh_buffer), ec);

			set_last_error(ec);

			return ec;
		}

		inline error_code set_dh_file(const std::string& dh_file) noexcept {
			error_code ec{};

			if (!dh_file.empty())
				this->use_tmp_dh_file(dh_file, ec);

			set_last_error(ec);

			return ec;
		}
	};
#endif

///////////////////////////kcp stream//////////////////////////////////////////////////////////////////////////
	template<class SocketType, class SvrOrCli>
	class kcp_stream : public SocketType {
    public:
        using socket_type = typename SocketType::socket_type;
        using stream_type = kcp_stream_tag;
	public:
		template<class ...Args>
		explicit kcp_stream(Args&&... args)
			: SocketType(std::forward<Args>(args)...)
			//, kcp_timer_(io.context())
		{
		}

		~kcp_stream() {
			if (this->kcp_) {
				kcp::ikcp_release(this->kcp_);
				this->kcp_ = nullptr;
			}
		}

		inline kcp::ikcpcb* kcp() {
			return this->kcp_;
		}

		inline auto& remote_endpoint() { return remote_endpoint_; }
        inline void remote_endpoint(const asio::ip::udp::endpoint& endpoint) { this->remote_endpoint_ = std::move(endpoint); }

		template<class SessionPtr>
		inline error_code kcp_handle_recv_t(SessionPtr&& ptr, std::string_view data) {
			//std::string_view data = std::string_view(static_cast<std::string_view::const_pointer>
			//		(ptr->buffer().rd_buf()), ptr->buffer().rd_size());
			// the kcp message header length is 24
			// the kcphdr length is 12 
			if (data.size() > kcp::kcphdr::required_size()) {
				return this->kcp_recv_t(ptr, data);
			}
			else if (data.size() == kcp::kcphdr::required_size()) {
				// Check whether the packet is SYN handshake
				// It is possible that the client did not receive the synack package, then the client
				// will resend the syn package, so we just need to reply to the syncack package directly.
				// If the client is disconnect without send a "fin" or the server has't recvd the 
				// "fin", and then the client connect again a later, at this time, the client
				// is in the session map already, and we need check whether the first message is fin
				if (kcp::is_kcphdr_syn(data)) {
					NET_ASSERT(this->kcp_);

					if (this->kcp_) {
						kcp::kcphdr syn = kcp::to_kcphdr(data);
						std::uint32_t conv = syn.th_ack;
						if (conv == 0) {
							conv = this->kcp_->conv;
							syn.th_ack = conv;
						}

						// If the client is forced disconnect after sent some messages, and the server
						// has recvd the messages already, we must recreated the kcp object, otherwise
						// the client and server will can't handle the next messages correctly.
					#if 0
						// set send_fin_ = false to make the _kcp_stop don't sent the fin frame.
						this->send_fin_ = false;

						this->kcp_stop_t();

						this->kcp_start_t(this_ptr, conv);
					#else
						this->kcp_reset_t();
					#endif

						this->send_fin_ = true;

						// every time we recv kcp syn, we sent synack to the client
						error_code ec;
						this->kcp_send_synack_t(syn, ec);
						if (ec) {
							//ptr->stop(ec);
							return ec;
						}
					}
					else {
						//ptr->stop(asio::error::operation_aborted);
						return asio::error::operation_aborted;
					}
				}
				else if (kcp::is_kcphdr_synack(data, 0, true)) {
					NET_ASSERT(this->kcp_);
				}
				else if (kcp::is_kcphdr_ack(data, 0, true)) {
					NET_ASSERT(this->kcp_);
				}
				else if (kcp::is_kcphdr_fin(data)) {
					NET_ASSERT(this->kcp_);

					this->send_fin_ = false;

					//ptr->stop(asio::error::connection_reset);
					return asio::error::connection_reset;
				}
				else {
					NET_ASSERT(false);
				}
			}
			else {
				NET_ASSERT(false);
			}
			return ec_ignore;
		}
	protected:
		template<class SessionPtr>
		asio::awaitable<void> kcp_start_t(SessionPtr&& ptr, std::uint32_t conv) {
			// used to restore configs
			kcp::ikcpcb* old = this->kcp_;

			struct old_kcp_destructor {
				old_kcp_destructor(kcp::ikcpcb* p) : p_(p) {}
				~old_kcp_destructor() {
					if (p_)
						kcp::ikcp_release(p_);
				}

				kcp::ikcpcb* p_ = nullptr;
			} old_kcp_destructor_guard(old);

			NET_ASSERT(conv != 0);

			this->kcp_ = kcp::ikcp_create(conv, (void*)this);
			this->kcp_->output = &kcp_stream::kcp_output_t;

			if (old) {
				// ikcp_setmtu
				kcp::ikcp_setmtu(this->kcp_, old->mtu);

				// ikcp_wndsize
				kcp::ikcp_wndsize(this->kcp_, old->snd_wnd, old->rcv_wnd);

				// ikcp_nodelay
				kcp::ikcp_nodelay(this->kcp_, old->nodelay, old->interval, old->fastresend, old->nocwnd);
			}
			else {
				kcp::ikcp_nodelay(this->kcp_, 1, 10, 2, 1);
				kcp::ikcp_wndsize(this->kcp_, 128, 512);
			}
			
			auto executor = co_await asio::this_coro::executor;
            asio::co_spawn(executor, [this, ptr, executor]() { 
				return this->kcp_timer_t(ptr, executor); 
			}, asio::detached);

			co_return;
		}

		void kcp_stop_t() {
			error_code ec_ignore{};

			// if is kcp mode, send FIN handshake before close
			if (this->send_fin_)
				this->kcp_send_hdr_t(kcp::make_kcphdr_fin(0), ec_ignore);

			//this->kcp_timer_.cancel();
		}

		inline void kcp_reset_t() {
			kcp::ikcp_reset(this->kcp_);
		}

		template<class SessionPtr>
		asio::awaitable<void> kcp_timer_t(SessionPtr&& ptr, const asio::any_io_executor& e) {
			asio::steady_timer ktimer(e, asio::chrono::seconds(0));
			while (/*ptr->is_started()*/ is_stream_start_ && this->socket().is_open()) {
				std::uint32_t clock1 = static_cast<std::uint32_t>(std::chrono::duration_cast<
					std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
				std::uint32_t clock2 = kcp::ikcp_check(this->kcp_, clock1);
				ktimer.expires_at(ktimer.expiry() + std::chrono::milliseconds(clock2 - clock1));
				asio::error_code ec;
          		co_await ktimer.async_wait(asio::redirect_error(asio::use_awaitable, ec));
				if (ec == asio::error::operation_aborted) {
					//ptr->stop(ec);
					co_return;
				}

				std::uint32_t clock = static_cast<std::uint32_t>(std::chrono::duration_cast<
					std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
				kcp::ikcp_update(this->kcp_, clock);
				if (this->kcp_->state == (kcp::IUINT32)-1) {
					ptr->stop(asio::error::network_reset);
					co_return;
				}
			}
			ktimer.cancel();
			co_return;
		}

		inline std::size_t kcp_send_hdr_t(kcp::kcphdr hdr, error_code& ec) {
			std::string msg = kcp::to_string(hdr);
			std::size_t sent_bytes = 0;

		//#if defined(_DEBUG) || defined(DEBUG)
		//	ASIO2_ASSERT(derive.post_send_counter_.load() == 0);
		//	derive.post_send_counter_++;
		//#endif

			if constexpr (is_svr_c<SvrOrCli>)
				sent_bytes = this->socket().send_to(asio::buffer(msg), this->remote_endpoint_, 0, ec);
			else
				sent_bytes = this->socket().send(asio::buffer(msg), 0, ec);

		//#if defined(_DEBUG) || defined(DEBUG)
		//	derive.post_send_counter_--;
		//#endif

			return sent_bytes;
		}

		inline std::size_t kcp_send_syn_t(std::uint32_t seq, error_code& ec) {
			kcp::kcphdr syn = kcp::make_kcphdr_syn(this->kcp_conv_, seq);
			return this->kcp_send_hdr_t(syn, ec);
		}

		inline std::size_t kcp_send_synack_t(kcp::kcphdr syn, error_code& ec) {
			// the syn.th_ack is the kcp conv
			kcp::kcphdr synack = kcp::make_kcphdr_synack(syn.th_ack, syn.th_seq);
			return this->kcp_send_hdr_t(synack, ec);
		}

		template<class SessionPtr>
		error_code kcp_recv_t(SessionPtr&& ptr, std::string_view data) {
			auto& buffer = ptr->buffer();

			int len = kcp::ikcp_input(this->kcp_, (const char *)data.data(), (long)data.size());

			buffer.reset();

			if (len != 0) {
				set_last_error(asio::error::message_size);

				//ptr->stop(asio::error::message_size);

				return asio::error::message_size;
			}

			for (;;) {
				len = kcp::ikcp_recv(this->kcp_, (char*)buffer.wr_buf(), buffer.wr_size());
				if (len >= 0) {
					// 通知逻辑层数据
					ptr->cbfunc()->call(event::recv, ptr, std::string_view(reinterpret_cast<
						std::string_view::const_pointer>(buffer.rd_buf()), len));
					//derive._fire_recv(this_ptr, ecs, std::string_view(static_cast
					//	<std::string_view::const_pointer>(buffer.data().data()), len));
				}
				else if (len == -3) {
					buffer.wr_reserve(init_buffer_size_);
					//buffer.pre_size((std::min)(buffer.pre_size() * 2, buffer.max_size()));
				}
				else {
					break;
				}
			}

			kcp::ikcp_flush(this->kcp_);

			return ec_ignore;
		}

		static int kcp_output_t(const char *buf, int len, kcp::ikcpcb *kcp, void *user) {
			std::ignore = kcp;

			kcp_stream * kself = ((kcp_stream*)user);

			error_code ec;
			if constexpr (is_svr_c<SvrOrCli>)
				kself->socket().send_to(asio::buffer(buf, len),
					kself->remote_endpoint_, 0, ec);
			else
				kself->socket().send(asio::buffer(buf, len), 0, ec);

			return 0;
		}

		template<class SessionPtr> 
		inline void handle_handshake(SessionPtr&& ptr) {
			ptr->cbfunc()->call(event::handshake, ptr, ec_ignore);
		}

		template<class SessionPtr>
		asio::awaitable<error_code> kcp_handshake_t(SessionPtr&& ptr) requires is_svr_c<SvrOrCli> {
			error_code ec;

			// first data
			std::string_view data = std::string_view(reinterpret_cast<
							std::string_view::const_pointer>(ptr->buffer().rd_buf()), ptr->buffer().rd_size());

			// step 3 : server recvd syn from client (the first data is the syn)
			kcp::kcphdr syn = kcp::to_kcphdr(data);
			std::uint32_t conv = syn.th_ack;
			if (conv == 0) {
				conv = this->kcp_conv_;
				syn.th_ack = conv;
			}

			// step 4 : server send synack to client
			this->kcp_send_synack_t(syn, ec);

			if (ec) {
				this->handle_handshake(ptr);
				co_return ec;
			}

			co_await this->kcp_start_t(ptr, conv);

			this->handle_handshake(ptr);
			co_return ec_ignore;
		}

		template<class SessionPtr>
		asio::awaitable<error_code> kcp_handshake_t (SessionPtr&& ptr) requires is_cli_c<SvrOrCli> {
			error_code ec;

			// step 1 : client send syn to server
			std::uint32_t seq = static_cast<std::uint32_t>(
				std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()).count());

			this->kcp_send_syn_t(seq, ec);

			if (ec) {
				//ptr->stop(ec);
				co_return ec;
			}

			// use a loop timer to execute "client send syn to server" until the server
			// has recvd the syn packet and this client recvd reply.
			auto is_canceled = std::make_shared<bool>(false);
			auto executor = co_await asio::this_coro::executor;
            asio::co_spawn(executor, [this, ptr, seq, executor, is_canceled]() -> asio::awaitable<void> { 	
				while(!is_canceled) {
					co_await async_sleep(executor, std::chrono::milliseconds(500), asio::use_awaitable);
					error_code ec;
					this->kcp_send_syn_t(seq, ec);
					if (ec) {
						ptr->stop(ec);
						co_return;
					}
				}
			}, asio::detached);

		//#if defined(_DEBUG) || defined(DEBUG)
		//	NET_ASSERT(derive.post_recv_counter_.load() == 0);
		//	derive.post_recv_counter_++;
		//#endif

			// step 2 : client wait for recv synack util connect timeout or recvd some data
			auto [rcvec, nrecv] = co_await ptr->socket().async_receive(asio::mutable_buffer(ptr->buffer().wr_buf(), ptr->buffer().wr_size()), 
                                            asio::as_tuple(asio::use_awaitable));
			*is_canceled = true;
            if (rcvec) {
				// if connect_timeout_timer_ is empty, it means that the connect timeout timer is
				// timeout and the callback has called already, so reset the error to timed_out.
				// note : when the async_resolve is failed, the socket is invalid to.
				this->handle_handshake(ptr);
                co_return rcvec; //asio::error::timed_out;
            }

			//ptr->buffer().rd_flip(nrecv);
			std::string_view data = std::string(reinterpret_cast<
							std::string_view::const_pointer>(ptr->buffer().rd_buf()), nrecv);

			// Check whether the data is the correct handshake information
			if (kcp::is_kcphdr_synack(data, seq)) {
				kcp::kcphdr hdr = kcp::to_kcphdr(data);
				std::uint32_t conv = hdr.th_seq;
				if (this->kcp_conv_ != 0) {
					NET_ASSERT(derive.kcp_conv_ == conv);
				}
				co_await this->kcp_start_t(ptr, conv);
				this->handle_handshake(ptr);
				co_return ec_ignore;
			}
			else {
				this->handle_handshake(ptr);
				co_return asio::error::address_family_not_supported;
			}
		}

		template<class SessionPtr>
		inline asio::awaitable<error_code> stream_start([[maybe_unused]] SessionPtr&& ptr) {
			try {
				is_stream_start_ = true;
				co_return co_await this->kcp_handshake_t(ptr);
			}
			catch (system_error& e) {
				set_last_error(e);
				co_return e.code();
			}
			co_return ec_ignore;
		}

		template<class SessionPtr>
		inline asio::awaitable<error_code> stream_stop([[maybe_unused]] SessionPtr&& ptr) {
			is_stream_start_ = false;
			kcp_stop_t();
			if (is_cli_c<SvrOrCli>) {
				SocketType::close();
			}
			co_return ec_ignore;
		}

	protected:
		//asio::steady_timer kcp_timer_;

		//nio& kcp_io_;
		std::uint32_t kcp_conv_ = 0;

		kcp::ikcpcb* kcp_ = nullptr;

		std::uint32_t seq_ = 0;

		bool send_fin_ = true;

		asio::ip::udp::endpoint remote_endpoint_;

		const std::size_t init_buffer_size_ = 1024;

		bool is_stream_start_ = false;
	};
}