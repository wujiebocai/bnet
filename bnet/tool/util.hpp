/*
 * Copyright (c) 2023 bocai
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

namespace bnet::tool {
	template <typename Enum>
	inline constexpr auto enum_to_int(Enum const value) -> typename std::underlying_type<Enum>::type {
		return static_cast<typename std::underlying_type<Enum>::type>(value);
	}

	// Returns true if the current machine is little endian
	template<typename = void>
	inline bool is_little_endian() noexcept {
		static std::int32_t test = 1;
		return (*reinterpret_cast<std::int8_t*>(std::addressof(test)) == 1);
	}

	/**
	 * Swaps the order of bytes for some chunk of memory
	 * @param data - The data as a uint8_t pointer
	 * @tparam DataSize - The true size of the data
	 */
	template <std::size_t DataSize>
	inline void swap_bytes(std::uint8_t * data) noexcept {
		for (std::size_t i = 0, end = DataSize / 2; i < end; ++i)
			std::swap(data[i], data[DataSize - i - 1]);
	}

	template<class T, class Pointer>
	inline void write(Pointer& p, T v) noexcept {
		if constexpr (int(sizeof(T)) > 1) {
			// MSDN: The htons function converts a u_short from host to TCP/IP network byte order (which is big-endian).
			// ** This mean the network byte order is big-endian **
			if (is_little_endian()) {
				swap_bytes<sizeof(T)>(reinterpret_cast<std::uint8_t *>(std::addressof(v)));
			}

			std::memcpy((void*)p, (const void*)&v, sizeof(T));
		}
		else {
			static_assert(sizeof(T) == std::size_t(1));

			*p = std::decay_t<std::remove_pointer_t<std::remove_cv_t<std::remove_reference_t<Pointer>>>>(v);
		}

		p += sizeof(T);
	}

	template<class T, class Pointer>
	inline T read(Pointer& p) noexcept {
		T v{};

		if constexpr (int(sizeof(T)) > 1) {
			std::memcpy((void*)&v, (const void*)p, sizeof(T));

			// MSDN: The htons function converts a u_short from host to TCP/IP network byte order (which is big-endian).
			// ** This mean the network byte order is big-endian **
			if (is_little_endian()) {
				swap_bytes<sizeof(T)>(reinterpret_cast<std::uint8_t *>(std::addressof(v)));
			}
		}
		else {
			static_assert(sizeof(T) == std::size_t(1));

			v = T(*p);
		}

		p += sizeof(T);

		return v;
	}

	inline std::size_t default_concurrency() noexcept {
		std::size_t num_threads = std::thread::hardware_concurrency() * 2;
		num_threads = num_threads == 0 ? 2 : num_threads;
		return num_threads;
	}
}

namespace std {
	/**
	 * BKDR Hash Function
	 */
	inline std::size_t bkdr_hash(const unsigned char* const p, std::size_t size) {
		std::size_t v = 0;
		for (std::size_t i = 0; i < size; ++i) {
			v = v * 131 + static_cast<std::size_t>(p[i]);
		}
		return v;
	}
	/**
	 * Fnv1a Hash Function
	 * Reference from Visual c++ implementation, see vc++ std::hash
	 */
	template<typename T>
	inline T fnv1a_hash(const unsigned char* const p, const T size) noexcept {
		static_assert(sizeof(T) == 4 || sizeof(T) == 8, "Must be 32 or 64 digits");
		T v;
		if constexpr (sizeof(T) == 4)
			v = 2166136261u;
		else
			v = 14695981039346656037ull;
		for (T i = 0; i < size; ++i) {
			v ^= static_cast<T>(p[i]);
			if constexpr (sizeof(T) == 4)
				v *= 16777619u;
			else
				v *= 1099511628211ull;
		}
		return (v);
	}
	/*
	template<> struct hash<asio::ip::udp::endpoint>
	{
		typedef asio::ip::udp::endpoint argument_type;
		typedef std::size_t result_type;
		inline result_type operator()(argument_type const& s) const noexcept
		{
			//return std::hash<std::string_view>()(std::string_view{
			//	reinterpret_cast<std::string_view::const_pointer>(&s),sizeof(argument_type) });
			return bkdr_hash((const unsigned char*)(&s), sizeof(argument_type));
		}
	};

	template<> struct hash<asio::ip::tcp::endpoint>
	{
		typedef asio::ip::tcp::endpoint argument_type;
		typedef std::size_t result_type;
		inline result_type operator()(argument_type const& s) const noexcept
		{
			//return std::hash<std::string_view>()(std::string_view{
			//	reinterpret_cast<std::string_view::const_pointer>(&s),sizeof(argument_type) });
			return bkdr_hash((const unsigned char*)(&s), sizeof(argument_type));
		}
	};
	*/
}

