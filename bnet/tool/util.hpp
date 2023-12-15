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
