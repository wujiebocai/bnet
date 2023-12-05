/*
 * Copyright (c) 2023 bocai
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include <deque>
#include "tool/tree.hpp"

namespace bnet::beast::http {
	class http_router {
	public:
		using handle_func_type =  std::function<void(http::web_request& req, http::web_response& rep)>;
		using redix_tree_type = typename tool::radix_tree<handle_func_type>;
    public:
        inline bool handle_func(http::verb method, const std::string& path, handle_func_type& func) {
			if (method <= http::verb::unknown || method > http::verb::unlink) {
				return false;
			}

			return routers_[size_t(method) - 1].add(path, std::forward<handle_func_type>(func));
		}

		inline std::optional<redix_tree_type::node_value> handle(http::verb method, const std::string& path) {
			if (method <= http::verb::unknown || method > http::verb::unlink) {
				return std::nullopt;
			}

			this->params_.clear();
			this->skipped_nodes_.clear();
        	return routers_[size_t(method) - 1].get(path, &params_, skipped_nodes_, false);
		}

		inline auto& params() { return params_; }
		inline auto& skipped_nodes() { return skipped_nodes_; }
    protected:
		std::vector<typename redix_tree_type::param> params_;
		std::vector<typename redix_tree_type::skipped_node> skipped_nodes_;

        std::array<redix_tree_type, size_t(http::verb::unlink)> routers_;
    };

	class http_cli_router {
	public:
		using handle_func_type =  std::function<void(const error_code& ec, http::web_response& rep)>;

		inline void handle_func(const handle_func_type& func) {
			handle_func_queue_.push_front(func);
		}

		template<class ... Args>
		inline bool handle(Args&&... args) {
			if (handle_func_queue_.empty()) {
				return false;
			}
			auto& func = handle_func_queue_.back();
			if (func) {
				func(std::forward<Args>(args)...);
			}
			handle_func_queue_.pop_back();
			return true;
		}

		template<class ... Args>
		inline void handle_all(Args&&... args) {
			for (auto& f : handle_func_queue_) {
				if (f) {
					f(std::forward<Args>(args)...);
				}
			}
			handle_func_queue_.clear();
		}

		inline size_t size() { return handle_func_queue_.size(); }
	
	protected:
		std::deque<handle_func_type> handle_func_queue_;
	};
}