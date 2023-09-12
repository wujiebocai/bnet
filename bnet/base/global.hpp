/*
 * Copyright (c) 2023 bocai
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include "base/session_mgr.hpp"

namespace bnet::base {
    // Global environment variable
    template<typename SessionType>
    struct global_val {
        session_mgr<SessionType> sessions_;

#if defined(BNET_ENABLE_HTTP)
        using handle_func_type = typename http::http_router::handle_func_type;
        
        http::http_router router_;
        
        inline bool handle_func(http::verb method, const std::string& path, handle_func_type& func) {
            return router_.handle_func(method, path, func);
        }

        inline auto handle(http::verb method, const std::string& path) {
            return router_.handle(method, path);
        }
#endif
    };
}