#pragma once

//#include <string>
//#include <functional>
//#include <memory>

//#include <locale>
//#include <codecvt>

//#include <iostream>

//#include "tool/util.hpp"

// https://github.com/gin-gonic/gin  tree.go

namespace bnet::tool {
    enum class node_type : std::int8_t {
		nt_static,      // 静态节点，比如上面的s，earch等节点
		nt_root,        // 根节点
		nt_param,       // 参数节点
		nt_catchall,    // 有*匹配的节点
	};

    //using handle_func =  std::function<std::string()>;

    template<typename handle_func>
    class radix_tree {
    public:
        struct node {
            // 节点路径
            std::string path_;
            // 和children字段对应, 保存的是分裂的分支的第一个字符
            // 例如search和support, 那么s节点的indices对应的"eu"
            // 代表有两个分支, 分支的首字母分别是e和u
            std::string indeces_;
            //节点是否是参数节点
            bool wild_child_;
            // 节点类型
            node_type nt_;
            // 优先级，子节点注册的handler数量
            std::uint32_t priority_;
            // 子节点，参数类型节点，最多一个子节点
            std::vector<std::shared_ptr<node>> children_;
            // 处理函数
            handle_func handlers_;
            // 路径上最大参数个数
            std::string fullPath_;
        };

        struct param {
            std::string key_ = "";
            std::string value_ = "";
        };

        struct node_value {
            handle_func handlers_ = nullptr;
            std::vector<param>* params_ = nullptr;
            bool tsr_ = false;
            std::string fullpath_ = "";
        };

        struct skipped_node {
            std::string path_ = "";
            std::shared_ptr<node> node_ = nullptr;
            size_t params_count_ = 0;
        };
    public:
        radix_tree()
            : root_(std::make_shared<node>()) {}

        inline bool add(const std::string& path, handle_func&& func) {
            return add_route(root_, path, func);
        }

        inline std::shared_ptr<node_value> get(const std::string& path, std::vector<param>* params, std::vector<skipped_node>& skipped_nodes, bool unescape) {
            return std::make_shared<node_value>(std::move(get_value(root_, path, params, skipped_nodes, unescape)));
        }

        inline const auto& root() { return root_;}
    protected:
        bool add_route(std::shared_ptr<node> n, std::string path, handle_func func) {
            std::string fullpath(path);
            n->priority_++;

            // empty tree
            auto npathsize = n->path_.size();
            auto nchildsize = n->children_.size();
            if (npathsize == 0 && nchildsize == 0) {
                n->nt_ = node_type::nt_root;
                return insert_child(n, path, fullpath, func);
            }

            auto parent_full_path_index = 0;

            while (true) {
walk:
                // Find the longest common prefix.
		        // This also implies that the common prefix contains no ':' or '*'
		        // since the existing key can't contain those chars.
                auto i = longest_common_prefix(path, n->path_);

                // Split edge
                if (i < n->path_.size()) {
                    auto child = std::make_shared<node>(n->path_.substr(i), n->indeces_, n->wild_child_, node_type::nt_static, n->priority_ - 1, n->children_, n->handlers_, n->fullPath_);

                    n->children_ = {child};
                    // []byte for proper unicode char conversion, see #65
                    //auto bpath = (std::uint8_t*)(n->path_.data());
                    n->indeces_ = n->path_[i];
                    n->path_ = path.substr(0, i);
                    n->handlers_ = nullptr;
                    n->wild_child_ = false;
                    n->fullPath_ = fullpath.substr(parent_full_path_index + i);
                }

                // Make new node a child of this node
                if (i < path.size()) {
                    path = path.substr(i);
                    auto c = path[0];

                    // '/' after param
                    if (n->nt_ == node_type::nt_param && c == '/' && n->children_.size() == 1) {
                        parent_full_path_index += n->path_.size();
                        n = n->children_[0];
                        n->priority_++;
                        goto walk;
                    }

                    // Check if a child with the next path byte exists
                    for (int i= 0, max = n->indeces_.size(); i < max; i++){
                        if (c == n->indeces_[i]) {
                            parent_full_path_index += n->path_.size();
                            i = increment_child_prio(n, i);
                            n = n->children_[i];
                            goto walk;
                        }
                    } 

                    // Otherwise insert it
                    if (c != ':' && c != '*' && n->nt_ != node_type::nt_catchall) {
                        // []byte for proper unicode char conversion, see #65
                        n->indeces_ += c;
                        auto child = std::make_shared<node>();
                        child->fullPath_ = fullpath;
                        add_child(n, child);
                        increment_child_prio(n, n->indeces_.size() -1);
                        n = child;
                    }
                    else if (n->wild_child_) {
                        // inserting a wildcard node, need to check if it conflicts with the existing wildcard
                        n = n->children_.back();
                        n->priority_++;

                        // Check if the wildcard matches
                        if (path.size() >= n->path_.size() && n->path_ == path.substr(0, n->path_.size()) &&
                            // Adding a child to a catchAll is not possible
                            n->nt_ != node_type::nt_catchall &&
                            // Check for longer wildcard, e.g. :name and :names
                            (n->path_.size() >= path.size() || path[n->path_.size()] == '/')) {
                                goto walk;
                        }

                        // Wildcard conflict
                        auto path_seg = path;
                        if (n->nt_ != node_type::nt_catchall) {
                            path_seg = tool::split(path_seg, "/", 2)[0];
                        }
                        auto idx = tool::ifind(fullpath, path_seg);
                        auto prefix = fullpath.substr(0, idx) + n->path_;
                        std::string panic_str = path_seg + 
                            " in new path " + fullpath +  
                            " conflicts with existing wildcard " + n->path_ + 
                            " in existing prefix " + prefix;
                        std::cerr << panic_str << std::endl;
                        return false;
                    }
  
                    return insert_child(n, path, fullpath, func);;
                }

                // Otherwise add handle to current node
                if (n->handlers_ != nullptr) {
                    std::string panic_str = "handlers are already registered for path " + fullpath;
                    std::cerr << panic_str << std::endl;
                    return false;
                }
                n->handlers_ = func;
                n->fullPath_ = fullpath;
                return true;
            }
            
        }

        template <typename T>
        void swap_obj(T &a, T &b) {
            T temp = std::move(a);
            a = std::move(b);
            b = std::move(temp);
        }

        int increment_child_prio(std::shared_ptr<node> n, int pos) {
            auto& cs = n->children_;
            cs[pos]->priority_++;
            auto prio = cs[pos]->priority_;

            // Adjust position (move to front)
            auto newpos = pos;
            for (; newpos > 0 && cs[newpos-1]->priority_ < prio; newpos--) {
                // Swap node positions
                std::swap(cs[newpos-1], cs[newpos]);
            }
            
            // Build new index char string
            if (newpos != pos) {
                n->indeces_ = n->indeces_.substr(0, newpos) + // Unchanged prefix, might be empty
                    n->indeces_.substr(pos, 1) + // The index char we move
                    n->indeces_.substr(newpos, pos-newpos) + n->indeces_.substr(pos+1); // Rest without char at 'pos'
            }

            return newpos;
        }

        // addChild will add a child node, keeping wildcardChild at the end
        void add_child(std::shared_ptr<node> n, std::shared_ptr<node> child) {
            if (n->wild_child_ && n->children_.size() > 0) {
                n->children_.emplace(n->children_.end() - 1, child);
            }
            else {
                n->children_.emplace_back(child);
            }
        }

        struct wildcard {
            std::string wildcard_;
            int i_;
            bool valid_;
        };

        // Search for a wildcard segment and check the name for invalid characters.
        // Returns -1 as index, if no wildcard was found.
        wildcard find_wildcard(std::string_view path) {
            wildcard wc;
            // Find start
            for(size_t sidx = 0; sidx < path.size(); sidx++) {
                // A wildcard starts with ':' (param) or '*' (catch-all)
                auto c = path[sidx];
                if (c != ':' && c != '*') {
                    continue;
                }

                // Find end and check for invalid characters
                wc.valid_ = true;
                for (size_t eidx = sidx + 1; eidx < path.size(); eidx++) {
                    auto c = path[eidx];
                    if (c == '/') {
                        wc.wildcard_ = path.substr(sidx, eidx - sidx);
                        wc.i_ = sidx;
                        return wc;
                    }
                    else if (c == ':' || c == '*') {
                        wc.valid_ = false;
                    }
                }
                
                wc.wildcard_ = path.substr(sidx);
                wc.i_ = sidx;
                return wc;
            }

            wc.wildcard_ = "";
            wc.i_ = -1;
            wc.valid_ = false;
            return wc;
        }

        bool insert_child(std::shared_ptr<node> n, std::string path, std::string fullpath, handle_func func) {
            while (true) {
                auto [wildcard, i, valid] = find_wildcard(path);
                if (i < 0) { // No wildcard found
                    break;
                }

                // The wildcard name must only contain one ':' or '*' character
                if (!valid) {
                    std::string panic_str = "only one wildcard per path segment is allowed, has: " + wildcard + " in path " + fullpath;
                    std::cerr << panic_str << std::endl;
                    return false;
                }

                // check if the wildcard has a name
                if (wildcard.size() < 2) {
                    std::string panic_str = "wildcards must be named with a non-empty name in path " + fullpath;
                    std::cerr << panic_str << std::endl;
                    return false;
                }

                if (wildcard[0] == ':') { // param
                    if (i > 0) {
                        // Insert prefix before the current wildcard
                        n->path_ = path.substr(0, i);
                        path = path.substr(i);
                    }
                    
                    auto child = std::make_shared<node>();
                    child->fullPath_ = fullpath;
                    child->nt_ = node_type::nt_param;
                    child->path_ = wildcard;
                    add_child(n, child);
                    n->wild_child_ = true;
                    n = child;
                    n->priority_++;

                    // if the path doesn't end with the wildcard, then there
			        // will be another subpath starting with '/'
                    if (wildcard.size() < path.size()) {
                        path = path.substr(wildcard.size());

                        auto child = std::make_shared<node>();
                        child->priority_ = 1;
                        child->fullPath_ = fullpath;
                        add_child(n, child);
                        n = child;
                        continue;
                    }

                    // Otherwise we're done. Insert the handle in the new leaf
                    n->handlers_ = func;
                    return true;
                }

                // catchAll
                if (i+wildcard.size() != path.size()) {
                    std::string panic_str = "catch-all routes are only allowed at the end of the path in path " + fullpath;
                    std::cerr << panic_str << std::endl;
                    return false;
                }

                if (n->path_.size() > 0 && n->path_.back() == '/') {
                    auto path_seg = tool::split(n->children_[0]->path_, "/", 2)[0];
                    std::string panic_str = "catch-all wildcard " + path + 
                        " in new path " + fullpath +
                        " conflicts with existing path segment " + path_seg +
                        " in existing prefix " + n->path_ + path_seg;
                    std::cerr << panic_str << std::endl;
                    return false;
                }

                // currently fixed width 1 for '/'
                i--;
                if (path[i] != '/') {
                    std::string panic_str = "no / before catch-all in path " + fullpath;
                    std::cerr << panic_str << std::endl;
                    return false;
                }

                n->path_ = path.substr(0, i);

                // First node: catchAll node with empty path
                auto child = std::make_shared<node>();
                child->wild_child_ = true;
                child->nt_ = node_type::nt_catchall;
                child->fullPath_ = fullpath;

                add_child(n, child);
                n->indeces_ = '/';
                n = child;
                n->priority_++;

                // second node: node holding the variable
                child = std::make_shared<node>();
                child->path_ = path.substr(i);
                child->nt_ = node_type::nt_catchall;
                child->handlers_ = func;
                child->priority_ = 1;
                child->fullPath_ = fullpath;

                n->children_ = {child};

                return true;
            }
            
            // If no wildcard was found, simply insert the path and handle
            n->path_ = path;
            n->handlers_ = func;
            n->fullPath_ = fullpath;

            return true;
        }

        size_t longest_common_prefix(std::string a, std::string b) {
            size_t i = 0;
            auto max = std::min(a.size(), b.size());
            while (i < max && a[i] == b[i]) {
                i++;
            }
            return i;
        }

        node_value get_value(std::shared_ptr<node> n, std::string path, std::vector<param>* params, std::vector<skipped_node>& skipped_nodes, bool unescape) {
            std::int16_t global_params_count = 0;
            node_value value;
            while (true) {
            walk: // Outer loop for walking the tree
                auto prefix = n->path_;
                if (path.size() > prefix.size()) {
                    if (path.substr(0, prefix.size()) == prefix) {
                        path = path.substr(prefix.size());

                        // Try all the non-wildcard children first by matching the indices
                        auto idxc = path[0];
                        for (size_t i = 0; i < n->indeces_.size(); i++) {
                            if (n->indeces_[i] == idxc) {
                                //  strings.HasPrefix(n.children[len(n.children)-1].path, ":") == n.wildChild
                                if (n->wild_child_) {
                                    auto nd = std::make_shared<node>();
                                    nd->path_ = n->path_;
                                    nd->wild_child_ = n->wild_child_;
                                    nd->nt_ = n->nt_;
                                    nd->priority_ = n->priority_;
                                    nd->children_ = n->children_;
                                    nd->handlers_ = n->handlers_;
                                    nd->fullPath_ = n->fullPath_;

                                    skipped_nodes.emplace_back(prefix+path, nd, global_params_count);
                                }

                                n = n->children_[i];
                                goto walk;
                            }
                        }

                        if (!n->wild_child_) {
                            // If the path at the end of the loop is not equal to '/' and the current node has no child nodes
					        // the current node needs to roll back to last valid skippedNode
                            if (path != "/") {
                                for (auto i = skipped_nodes.size(); i > 0; i--) {
                                    auto skipped_node = skipped_nodes[i - 1];
                                    skipped_nodes.pop_back();
                                    if (has_suffix(skipped_node.path_, path)) {
                                        path = skipped_node.path_;
                                        n = skipped_node.node_;
                                        if (value.params_ != nullptr) {
                                            if ((*value.params_).size() > skipped_node.params_count_) {
                                                auto beginitr = (*value.params_).begin() + skipped_node.params_count_;
                                                auto enditr = (*value.params_).end();
                                                (*value.params_).erase(beginitr, enditr);
                                            }
                                            else {
                                                //(*value.params_).clear();
                                            }
                                        }
                                        
                                        global_params_count = skipped_node.params_count_;
                                        goto walk;
                                    }
                                }
                            }
                            // Nothing found.
					        // We can recommend to redirect to the same URL without a
					        // trailing slash if a leaf exists for that path.
                            value.tsr_ = (path == "/" && n->handlers_ != nullptr);
                            return value;
                        }
                        
                        // Handle wildcard child, which is always at the end of the array
                        n = n->children_.back();
                        global_params_count++;

                        switch (n->nt_) {
                        case node_type::nt_param:
                            {
                                // fix truncate the parameter
					            // tree_test.go  line: 204

					            // Find param end (either '/' or path end)
                                size_t end = 0;
                                while (end < path.size() && path[end] != '/') {
                                    end++;
                                }
                            
                                // Save param value
                                if (params != nullptr) {
                                    if (value.params_ == nullptr) {
                                        value.params_ = params;
                                    }
                                    // Expand slice within preallocated capacity
                                    //auto i = (*value.params_).size();
                                    auto val = path.substr(0, end);
                                    if (unescape) {
                                       // 有待实现
                                    }
                                    (*value.params_).emplace_back(n->path_.substr(1), val);
                                }

                                // we need to go deeper!
                                if (end < path.size()) {
                                    if (n->children_.size() > 0) {
                                        path = path.substr(end);
                                        n = n->children_[0];
                                        goto walk;
                                    }

                                    // ... but we can't
                                    value.tsr_ = (path.size() == end + 1);
                                    return value;
                                }

                                if (value.handlers_ = n->handlers_; value.handlers_ != nullptr) {
                                    value.fullpath_ = n->fullPath_;
                                    return value;
                                }
                                if (n->children_.size() == 1) {
                                    // No handle found. Check if a handle for this path + a
						            // trailing slash exists for TSR recommendation
                                    n = n->children_[0];
                                    value.tsr_ = (n->path_ == "/" && n->handlers_ != nullptr) || (n->path_ == "" && n->indeces_ == "/");
                                }
                                return value;
                            }
                            break;

                        case node_type::nt_catchall:
                            {
                                // Save param value
                                if (params != nullptr) {
                                    if (value.params_ == nullptr) {
                                        value.params_ = params;
                                    }
                                    // Expand slice within preallocated capacity
                                    //auto i = (*value.params_).size();
                                    auto val = path;
                                    if (unescape) {
                                        // 有待实现
                                    }
                                    (*value.params_).emplace_back(n->path_.substr(2), val);
                                }

                                value.handlers_ = n->handlers_;
                                value.fullpath_ = n->fullPath_;
                                return value;
                            }
                            break;
                            
                        default:
                            std::string panic_str = "invalid node type";
                            std::cerr << panic_str << std::endl;
                            return value;
                        }
                    }
                } 

                if (path == prefix) {
                    // If the current path does not equal '/' and the node does not have a registered handle and the most recently matched node has a child node
			        // the current node needs to roll back to last valid skippedNode
                    if (n->handlers_ == nullptr && path != "/") {
                        for (auto i = skipped_nodes.size(); i > 0; i--) {
                            auto skipped_node = skipped_nodes.back();
                            skipped_nodes.pop_back();
                            if (has_suffix(skipped_node.path_, path)) {
                                path = skipped_node.path_;
                                n = skipped_node.node_;
                                if (value.params_ != nullptr) {
                                    if ((*value.params_).size() > skipped_node.params_count_) {
                                        auto beginitr = (*value.params_).begin() + skipped_node.params_count_;
                                        auto enditr = (*value.params_).end();
                                        (*value.params_).erase(beginitr, enditr);
                                    }
                                    else {
                                        //(*value.params_).clear();
                                    }
                                }
                                global_params_count = skipped_node.params_count_;
                                goto walk;
                            }
                        }
                        //	n = latestNode.children[len(latestNode.children)-1]
                    }
                    // We should have reached the node containing the handle.
			        // Check if this node has a handle registered.
                    if (value.handlers_ = n->handlers_; value.handlers_ != nullptr) {
                        value.fullpath_ = n->fullPath_;
                        return value;
                    }

                    // If there is no handle for this route, but this route has a
			        // wildcard child, there must be a handle for this path with an
			        // additional trailing slash
                    if (path == "/" && n->wild_child_ && n->nt_ != node_type::nt_root) {
                        value.tsr_ = true;
                        return value;
                    }

                    if (path == "/" && n->nt_ == node_type::nt_static) {
                        value.tsr_ = true;
                        return value;
                    }

                    // No handle found. Check if a handle for this path + a
			        // trailing slash exists for trailing slash recommendation
                    for (size_t i = 0; i < n->indeces_.size(); i++) {
                        if (n->indeces_[i] == '/') {
                            n = n->children_[i];
                            value.tsr_ = (n->path_.size() == 1 && n->handlers_ != nullptr) || (n->nt_ == node_type::nt_catchall && n->children_[0]->handlers_ != nullptr);
                            return value;
                        }
                    }

                    return value;
                }

                // Nothing found. We can recommend to redirect to the same URL with an
		        // extra trailing slash if a leaf exists for that path
                value.tsr_ = (path == "/") || 
                    (prefix.size() == path.size()+1 && prefix[path.size()] == '/' &&
                    path == prefix.substr(0, prefix.size() - 1) && n->handlers_ != nullptr);
                
                // roll back to last valid skippedNode
                if (!value.tsr_ && path != "/") {
                    for (auto i = skipped_nodes.size(); i > 0; i--) {
                        auto skipped_node = skipped_nodes.back();
                        skipped_nodes.pop_back();
                        if (has_suffix(skipped_node.path_, path)) {
                            path = skipped_node.path_;
                            n = skipped_node.node_;
                            if (value.params_ != nullptr) {
                                if ((*value.params_).size() > skipped_node.params_count_) {
                                    auto beginitr = (*value.params_).begin() + skipped_node.params_count_;
                                    auto enditr = (*value.params_).end();
                                    (*value.params_).erase(beginitr, enditr);
                                }
                                else {
                                    //(*value.params_).clear();
                                }
                            }
                            global_params_count = skipped_node.params_count_;
                            goto walk;
                        }
                    }
                }

                return value;
            }
        }

    private:
        std::shared_ptr<node> root_;
    };
}