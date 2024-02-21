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
	class http_server : public server<StreamType, ProtoType> {
    public:
        using super = server<StreamType, ProtoType>;
        using super::super;
        using handle_func_type = typename http::http_router::handle_func_type;
    public:
        inline bool get(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::get, path, func);
        }

        inline bool post(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::post, path, func);
        }

        inline bool del(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::delete_, path, func);
        }

        inline bool head(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::head, path, func);
        }

        inline bool put(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::put, path, func);
        }

        inline bool connect(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::connect, path, func);
        }

        inline bool options(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::options, path, func);
        }

        inline bool trace(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::trace, path, func);
        }

        inline bool copy(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::copy, path, func);
        }

        inline bool lock(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::lock, path, func);
        }

        inline bool mkcol(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::mkcol, path, func);
        }

        inline bool move(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::move, path, func);
        }

        inline bool propfind(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::propfind, path, func);
        }

        inline bool proppatch(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::proppatch, path, func);
        }

        inline bool search(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::search, path, func);
        }

        inline bool unlock(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::unlock, path, func);
        }

        inline bool hbind(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::bind, path, func);
        }

        inline bool rebind(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::rebind, path, func);
        }

        inline bool unbind(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::unbind, path, func);
        }

        inline bool acl(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::acl, path, func);
        }

        inline bool report(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::report, path, func);
        }

        inline bool mkactivity(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::mkactivity, path, func);
        }

        inline bool checkout(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::checkout, path, func);
        }

        inline bool merge(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::merge, path, func);
        }

        inline bool msearch(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::msearch, path, func);
        }

        inline bool notify(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::notify, path, func);
        }

        inline bool subscribe(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::subscribe, path, func);
        }

        inline bool unsubscribe(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::unsubscribe, path, func);
        }

        inline bool patch(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::patch, path, func);
        }

        inline bool purge(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::purge, path, func);
        }

        inline bool mkcalendar(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::mkcalendar, path, func);
        }

        inline bool link(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::link, path, func);
        }

        inline bool unlink(const std::string& path, handle_func_type func) {
            return this->globalctx_.handle_func(http::verb::unlink, path, func);
        }
    };
#endif
}