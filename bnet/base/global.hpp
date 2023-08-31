#pragma once

#include "base/session_mgr.hpp"

namespace bnet::base {
    template<typename SessionType>
    struct global_val {
        session_mgr<SessionType> sessions_;

#if defined(NET_USE_HTTP)
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