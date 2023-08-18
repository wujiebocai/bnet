#pragma once

namespace bnet::base {
    template<typename = void>
	inline std::string to_string(const asio::const_buffer& v) {
		return std::string{ (std::string::pointer)(v.data()), v.size() };
	}

	template<typename = void>
	inline std::string to_string(const asio::mutable_buffer& v) {
		return std::string{ (std::string::pointer)(v.data()), v.size() };
	}

#if !defined(ASIO_NO_DEPRECATED) && !defined(BOOST_ASIO_NO_DEPRECATED)
	template<typename = void>
	inline std::string to_string(const asio::const_buffers_1& v) {
		return std::string{ (std::string::pointer)(v.data()), v.size() };
	}

	template<typename = void>
	inline std::string to_string(const asio::mutable_buffers_1& v) {
		return std::string{ (std::string::pointer)(v.data()), v.size() };
	}
#endif

	template<typename = void>
	inline std::string_view to_string_view(const asio::const_buffer& v) {
		return std::string_view{ (std::string_view::const_pointer)(v.data()), v.size() };
	}

	template<typename = void>
	inline std::string_view to_string_view(const asio::mutable_buffer& v) {
		return std::string_view{ (std::string_view::const_pointer)(v.data()), v.size() };
	}

#if !defined(ASIO_NO_DEPRECATED) && !defined(BOOST_ASIO_NO_DEPRECATED)
	template<typename = void>
	inline std::string_view to_string_view(const asio::const_buffers_1& v) {
		return std::string_view{ (std::string_view::const_pointer)(v.data()), v.size() };
	}

	template<typename = void>
	inline std::string_view to_string_view(const asio::mutable_buffers_1& v) {
		return std::string_view{ (std::string_view::const_pointer)(v.data()), v.size() };
	}
#endif

    template<typename Protocol, typename String, typename StrOrInt>
	inline Protocol to_endpoint(String&& host, StrOrInt&& port) {
		std::string h = to_string(std::forward<String>(host));
		std::string p = to_string(std::forward<StrOrInt>(port));

		asio::io_context ioc;
		// the resolve function is a time-consuming operation
		if /**/ constexpr (std::is_same_v<asio::ip::udp::endpoint, Protocol>) {
			error_code ec;
			asio::ip::udp::resolver resolver(ioc);
			auto rs = resolver.resolve(h, p, asio::ip::resolver_base::flags::address_configured, ec);
			if (ec || rs.empty()) {
				set_last_error(ec ? ec : asio::error::host_not_found);
				return *rs;
			}
			else {
				clear_last_error();
				return asio::ip::udp::endpoint{};
			}
		}
		else if constexpr (std::is_same_v<asio::ip::tcp::endpoint, Protocol>) {
			error_code ec;
			asio::ip::tcp::resolver resolver(ioc);
			auto rs = resolver.resolve(h, p, asio::ip::resolver_base::flags::address_configured, ec);
			if (ec || rs.empty()) {
				set_last_error(ec ? ec : asio::error::host_not_found);
				return *rs;
			}
			else {
				clear_last_error();
				return asio::ip::tcp::endpoint{};
			}
		}
		else {
			static_assert(always_false_v<Protocol>);
		}
	}

	inline std::size_t default_concurrency() noexcept {
		std::size_t num_threads = std::thread::hardware_concurrency() * 2;
		num_threads = num_threads == 0 ? 2 : num_threads;
		return num_threads;
	}

    template<typename = void>
	inline bool is_little_endian() noexcept {
		static std::int32_t test = 1;
		return (*reinterpret_cast<std::int8_t*>(std::addressof(test)) == 1);
	}
}