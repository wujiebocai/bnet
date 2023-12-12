/*
 * Copyright (c) 2023 bocai
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

namespace bnet {
	enum class estate : std::int8_t { stopped, stopping, starting, started };

	enum class event : std::int8_t {
		init,
		connect,
		disconnect,
		recv,
		handshake,
		upgrade,
		max
	};

namespace base {
	using BINDFUNCTYPE = func_proxy_imp<event>;
	typedef std::shared_ptr<BINDFUNCTYPE> bind_func_ptr_type;
}
}

