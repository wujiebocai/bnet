/*
 * Copyright (c) 2023 bocai
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/
#pragma once

namespace bnet::base {
#if defined(BNET_ENABLE_HTTP)
	template<class StreamType, class ProtoType>
	class http_client : public client<StreamType, ProtoType> {
    public:
        using super = client<StreamType, ProtoType>;
        using super::super;
        using handle_func_type = typename http::http_cli_router::handle_func_type;
    public:
        template<class Body = http::string_body, class Fields = http::fields>
        inline bool execute(http::request<Body, Fields>& req, handle_func_type func) {
            auto sptr = this->globalctx_.sessions_.rand_get();
            if (!sptr) {
                return false;
            }

            sptr->send(req, func);

            return true;
        }

        template<class Body = http::string_body, class Fields = http::fields>
        inline bool execute(http::web_request& req, handle_func_type func) {
            auto sptr = this->globalctx_.sessions_.rand_get();
            if (!sptr) {
                return false;
            }

            sptr->send(req, func);

            return true;
        }

        template<class Body = http::string_body, class Fields = http::fields>
        inline bool execute(std::string_view url, handle_func_type func) {
            auto sptr = this->globalctx_.sessions_.rand_get();
            if (!sptr) {
                return false;
            }

            http::web_request req = http::make_request(url);
			if (get_last_error()) {
				return false;
			}

            sptr->send(req, func);

            return true;
        }
    };
#endif
}