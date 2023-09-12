/*
 * Copyright (c) 2023 bocai
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#if !defined(NDEBUG) && !defined(_DEBUG) && !defined(DEBUG)
#define NDEBUG
#endif

namespace bnet::error {
	enum http_errors {
  		/// Already open.
  		already_open = 1,

  		/// End of file or stream.
  		eof,

  		/// Element not found.
  		not_found,

  		/// The descriptor cannot fit into the select system call's fd_set.
  		fd_set_failure
	};

namespace detail {
	class http_category : public asio::error_category {
	public:
  		const char* name() const ASIO_ERROR_CATEGORY_NOEXCEPT {
    		return "asio.misc";
  		}

  		std::string message(int value) const {
    		if (value == error::already_open)
    		  return "Already open";
    		if (value == error::eof)
    		  return "End of file";
    		if (value == error::not_found)
    		  return "Element not found 123";
    		if (value == error::fd_set_failure)
    		  return "The descriptor does not fit into the select call's fd_set";
    		return "asio.misc error";
  		}
	};
}

	const asio::error_category& get_http_category() {
  		static detail::http_category instance;
  		return instance;
	}

	inline asio::error_code make_error_code(http_errors e) {
  		return asio::error_code(
      		static_cast<int>(e), get_http_category());
	}
}

namespace std {
	template<> struct is_error_code_enum<bnet::error::http_errors> {
  		static const bool value = true;
	};
}

namespace bnet {

	#ifdef NET_ASSERT
		static_assert(false, "Unknown NET_ASSERT definition will affect the relevant functions of this program.");
	#else
		#if defined(_DEBUG) || defined(DEBUG)
			#define NET_ASSERT(x) assert(x);
		#else
			#define NET_ASSERT(x) (void)0;
		#endif
	#endif

	using error_code = ::asio::error_code;
	using system_error = ::asio::system_error;
		
	thread_local static error_code ec_last;

	inline error_code & get_last_error() {
		return ec_last;
	}

	inline void set_last_error(int ec) {
		ec_last.assign(ec, asio::error::get_system_category());
	}

	template<typename T>
	inline void set_last_error(int ec, const T& ecat) {
		ec_last.assign(ec, ecat);
	}

	inline void set_last_error(const error_code & ec) {
		ec_last = ec;
	}

	inline void set_last_error(const system_error & e) {
		ec_last = e.code();
	}

	inline void clear_last_error() {
		ec_last.clear();
	}

	inline auto last_error_val() {
		return ec_last.value();
	}

	inline auto last_error_msg() {
		return ec_last.message();
	}

	thread_local static error_code ec_ignore;

	//asio::error::timed_out
	//asio::error::operation_aborted 
}
