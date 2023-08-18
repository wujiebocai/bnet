#include <iostream> 
#include "bnet.hpp"
#include "other.hpp"
#include "http/router.hpp"
#include <regex>
#include "tree_tst.hpp"

using namespace bnet;

asio::io_context g_context_(1);
asio::io_context::strand g_context_s_(g_context_);
const int timenum = 10;

void print_time() {
	// 获取当前时间
    time_t currentTime = time(0);

    // 将当前时间转换为本地时间
    struct tm* localTime = localtime(&currentTime);

    // 显示当前时间
    std::cout << "当前时间为: "
              << localTime->tm_year + 1900 << "-"
              << localTime->tm_mon + 1 << "-"
              << localTime->tm_mday << " "
              << localTime->tm_hour << ":"
              << localTime->tm_min << ":"
              << localTime->tm_sec << std::endl;
}

template<class SvrType>
class svr_proxy : public SvrType {
public:
	using session_ptr_type = typename SvrType::session_ptr_type;
public:
	svr_proxy(std::size_t concurrency = std::thread::hardware_concurrency() * 2, std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)()) 
		: SvrType(concurrency, max_buffer_size)
		, ctimer_(g_context_, asio::chrono::seconds(0)) {

		this->bind(event::connect, [&](session_ptr_type& ptr, error_code ec) {
			std::cout << "connect" << ec.message() << std::endl;
			//ptr->stop(ec);
		});
		this->bind(event::handshake, [](session_ptr_type& ptr, error_code ec) { // only ssl call back
			std::cout << "handshake" << ec.message() << std::endl;
		});
		this->bind(event::disconnect, [](session_ptr_type& ptr, error_code ec) {
			std::cout << "disconnect" << ec.message() << std::endl;
		});
		this->bind(event::recv, [&](session_ptr_type& ptr, std::string_view&& s) {
			//std::cout << s << count_ << std::endl;
			//count_.fetch_add(s.size());
			ptr->send(std::move(s));
			++count_;
		});
	}

	inline void test_timer() {
		asio::co_spawn(g_context_, [this]() { 
			return this->count_timer();
		}, asio::detached);
	}

    inline asio::awaitable<void> count_timer() {
        //asio::steady_timer timer(co_await asio::this_coro::executor);
        
        while (true) {
            //this->acceptor_timer_.expires_after(std::chrono::seconds(10));
			//ctimer_.expires_after(ctimer_.expiry() + std::chrono::seconds(10));
			ctimer_.expires_at(ctimer_.expiry() + asio::chrono::seconds(timenum));
            co_await ctimer_.async_wait(asio::use_awaitable);

			print_time();

            std::cout << count_ / timenum << " " << count_ << std::endl;
		    count_ = 0;
        }
    }

private:
	asio::steady_timer ctimer_;
	//bnet::Timer testtimer_;
	std::atomic<std::size_t> count_{ 0 };
};

template<class CliType>
class cli_proxy : public CliType {
public:
	using session_ptr_type = typename CliType::session_ptr_type;
public:
	cli_proxy(std::size_t concurrency = std::thread::hardware_concurrency() * 2, std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)())
		: CliType(concurrency, max_buffer_size) {

		this->bind(event::connect, [](session_ptr_type& ptr, error_code ec) {
			if (!ec) {
				std::string msgdata(1024, 'a');
				ptr->send("a");
			}
			std::cout << "client connect" << ec.message() << std::endl;
		});
		this->bind(event::handshake, [](session_ptr_type& ptr, error_code ec) { // only ssl call back
			std::cout << "client handshake" << ec.message() << std::endl;
		});
		this->bind(event::disconnect, [](session_ptr_type& ptr, error_code ec) {
			std::cout << "client disconnect" << ec.message() << std::endl;
			if (ec) {
				//ptr->reconn(); //测试异常退出重连
			}
			//ptr->reconn();
		});
		this->bind(event::recv, [](session_ptr_type& ptr, std::string_view&& s) {
			ptr->send(std::move(s));
		});
	}
};

bool matchPath(const std::string& route, const std::string& path) {
    std::regex wildcard("\\*");
    std::string processedRoute = std::regex_replace(route, wildcard, ".*");

    std::regex pattern(processedRoute);
    return std::regex_match(path, pattern);
}

class TrieNode {
public:
    bool isEndpoint;
    std::unordered_map<std::string, TrieNode*> children;

    TrieNode() : isEndpoint(false) {}
};

class Router {
private:
    TrieNode* root;

public:
    Router() {
        root = new TrieNode();
    }

    void addRoute(const std::string& route) {
        TrieNode* node = root;

        std::vector<std::string> paths = splitPath(route);

        for (const std::string& path : paths) {
            if (node->children.find(path) == node->children.end()) {
                node->children[path] = new TrieNode();
            }
            node = node->children[path];
        }

        node->isEndpoint = true;
    }

    bool matchPath(const std::string& path) {
        std::vector<std::string> paths = splitPath(path);
        return matchPathHelper(root, paths, 0);
    }

private:
    std::vector<std::string> splitPath(const std::string& path) {
        std::vector<std::string> paths;
        std::string delimiter = "/";
        size_t start = 0;
        size_t end = path.find(delimiter);

        while (end != std::string::npos) {
            std::string token = path.substr(start, end - start);
            paths.push_back(token);
            start = end + delimiter.length();
            end = path.find(delimiter, start);
        }

        std::string lastToken = path.substr(start, end);
        paths.push_back(lastToken);

        return paths;
    }

    bool matchPathHelper(TrieNode* node, const std::vector<std::string>& paths, std::size_t index) {
        if (index == paths.size()) {
            return node->isEndpoint;
        }

        if (node->children.find(paths[index]) != node->children.end()) {
            return matchPathHelper(node->children[paths[index]], paths, index + 1);
        }

        if (node->children.find("*") != node->children.end()) {
            return matchPathHelper(node->children["*"], paths, index + 1);
        }

        return false;
    }
};

int64_t get_cur_time() {
	auto currentTime = std::chrono::system_clock::now();

    // Convert the current time point to milliseconds since the epoch
    auto timeSinceEpoch = currentTime.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceEpoch).count();

	return milliseconds;
}

#include <unordered_set>
class RadixTreeNode {
public:
  explicit RadixTreeNode(const std::string &word = "", bool is_end = false)
      : word(word), is_end(is_end) {}

  std::unordered_set<std::shared_ptr<RadixTreeNode>> children;
  std::string word;
  bool is_end;
};

class RadixTree {
public:
  RadixTree() : root(std::make_shared<RadixTreeNode>()){};

  virtual ~RadixTree() = default;

  RadixTree(const RadixTree &) = delete;
  RadixTree &operator=(const RadixTree &) = delete;

  void insert(const std::string &str) {
    if (str.empty()) {
      root->is_end = true;
    } else {
      insert_helper(str, root);
    }
  }

  void erase(const std::string &str) {
    if (str.empty()) {
      root->is_end = false;
    } else {
      erase_helper(str, root);
    }
  }

  bool search(const std::string &str) {
    if (str.empty()) {
      return root->is_end;
    }
    return search_helper(str, root);
  }

private:
  std::shared_ptr<RadixTreeNode> root;

  void insert_helper(const std::string &str, std::shared_ptr<RadixTreeNode> node) {
    // 如果当前没有子节点，则直接作为新的子节点
    if (node->children.empty()) {
      auto new_node = std::make_shared<RadixTreeNode>(str, true);
      node->children.insert(new_node);
      return;
    }

    bool is_match = false;
    for (auto current : node->children) {
      size_t i = 0;
      for (; i < str.size() && i < current->word.size(); i++) {
        if (str[i] != current->word[i]) {
          break;
        }
      }
      if (i != 0) {
        is_match = true;
        // 情况一：当前节点的内容与字符串完全匹配，则直接将该前缀标记为完整
        if (i == str.size() && i == current->word.size()) {
          current->is_end = true;
        } else if (i != current->word.size()) {
          // 如果当前节点的内容是字符串的部分前缀，则进行分裂
          auto new_node = std::make_shared<RadixTreeNode>(current->word.substr(i),
                                                     current->is_end);

          current->word = current->word.substr(0, i);
          current->is_end = (i == str.size()) ? true : false;
          current->children.swap(new_node->children);
          current->children.insert(new_node);

          if (i != str.size()) {
            auto new_node2 = std::make_shared<RadixTreeNode>(str.substr(i), true);
            current->children.insert(new_node2);
          }
        } else {
          // 如果当前节点已匹配完，则继续往子节点匹配
          insert_helper(str.substr(i), current);
        }
        if (is_match) {
          return;
        }
      }
    }
    // 如果没有找到，则直接插入
    auto new_node = make_shared<RadixTreeNode>(str, true);
    node->children.insert(new_node);
  }

  std::shared_ptr<RadixTreeNode> erase_helper(const std::string &str,
                                         std::shared_ptr<RadixTreeNode> node) {
    bool is_match = false;
    for (auto current : node->children) {
      size_t i = 0;
      for (; i < str.size() && i < current->word.size(); i++) {
        if (str[i] != current->word[i]) {
          break;
        }
      }
      if (i != 0) {
        is_match = true;

        // 情况一：当前节点的内容与字符串完全匹配
        if (i == str.size() && i == current->word.size()) {
          // 如果该节点没有子节点，则将该节点删除。否则将is_end标记为false
          if (current->children.empty()) {
            node->children.erase(current);
          } else {
            current->is_end = false;
          }

          // 如果删除了该节点后，父节点仅剩下一个子节点，且父节点不完整，则将两个节点合并
          if (node->children.size() == 1 && !node->is_end && node != root) {
            auto sub_node = *node->children.begin();
            node->children.erase(sub_node);
            node->is_end = sub_node->is_end;
            node->word.append(sub_node->word);
            node->children = sub_node->children;
            return node;
          }
        }
        // 情况二：当前节点是字符串的前缀
        else if (i == current->word.size()) {
          // 继续向下搜索，如果返回值不为空则说明需要合并节点
          auto sub_node = erase_helper(str.substr(i), current);
          if (sub_node && node->children.size() == 1 && !node->is_end &&
              node != root) {
            auto sub_node = *node->children.begin();
            node->children.erase(sub_node);
            node->is_end = sub_node->is_end;
            node->word.append(sub_node->word);
            node->children = sub_node->children;
          }
        }
        // 情况三：字符串是当前节点的前缀，此时必定查询失败
        else {
          break;
        }
      }
      if (is_match) {
        return nullptr;
      }
    }
    return nullptr;
  }

  bool search_helper(const std::string &str, std::shared_ptr<RadixTreeNode> node) {
    for (auto current : node->children) {
      size_t i = 0;
      for (; i < str.size() && i < current->word.size(); i++) {
        if (str[i] != current->word[i]) {
          break;
        }
      }
      if (i != 0) {
        // 情况一：当前节点的内容与字符串完全匹配，根据是否为完整单词判断结果
        if (i == str.size() && i == current->word.size()) {
          return current->is_end;
        }
        // 情况二：当前节点的内容是字符串的前缀
        else if (i == current->word.size()) {
          return search_helper(str.substr(i), current);
        }
        // 情况三：字符串的内容是当前节点的前缀，直接返回错误
        else {
          return false;
        }
      }
    }
    // 没有找到
    return false;
  }
};

int main(int argc, char * argv[]) {
	/*
	int newpos = 2, pos = 3;
	std::string indeces_ = "csufd";
    auto f1 = indeces_.substr(0, newpos);
    auto f2 = indeces_.substr(pos, 1);
    auto f3 = indeces_.substr(newpos, pos-newpos);
    auto f4 = indeces_.substr(newpos);
	*/
	/*
	std::string pathtst = "aaaaa/sss/ss";
	auto strs = bnet::split(pathtst, "/", -1);
	strs = bnet::split(pathtst, "/", 0);
	strs = bnet::split(pathtst, "/", 1);
	strs = bnet::split(pathtst, "/", 2);
	strs = bnet::split(pathtst, "/", 3);
	strs = bnet::split(pathtst, "/", 4);
	auto pathSeg = strs[0];
	return 0;
	*/

	auto start_time = get_cur_time();

  for (int i = 0; i < 100; i++) {
    test_rount ();
  }

  auto end_time1 = get_cur_time();
	std::cout << "Path 1 matches the route. cost time:" << (end_time1 - start_time) << std::endl;
	return 0;

	//std::string mroute = "/users/*/profile";
	//std::string mpath1 = "/users/john1/profile";
	//for (int i = 0; i < 10000; i++) {
	//	if (matchPath(mroute, mpath1)) {
    //    	//std::cout << "Path 1 matches the route." << std::endl;
    //	}
	//}
	
	//Router mrouter;
	//mrouter.addRoute("/users/*/profile");
	//for (int i = 0; i < 10000; i++) {
	//	if (mrouter.matchPath("/users/john/profile")) {
	//	}
	//}

	RadixTree mrouter;
	// 添加路由路径
    mrouter.insert("/home");
    mrouter.insert("/user/:id");
    mrouter.insert("/post/*");

    // 查找匹配的路径
    std::cout << mrouter.search("/home") << std::endl;        // 输出: /home
    std::cout << mrouter.search("/user/123") << std::endl;    // 输出: /user/:id
    std::cout << mrouter.search("/post/123/comments") << std::endl; // 输出: /post/*
    std::cout << mrouter.search("/about") << std::endl;       // 输出: 空字符串（未找到匹配的路径）

	auto end_time = get_cur_time();
	std::cout << "Path 1 matches the route. cost time:" << (end_time - start_time) << std::endl;
	return 0;

	Router router;

    // 添加路由
    router.addRoute("/users/*/profile");
    //router.addRoute("/users/admin/profile");
    router.addRoute("/posts/*");

    // 测试路径匹配
    std::cout << std::boolalpha;
    std::cout << router.matchPath("/users/john/profile") << std::endl;  // true
    std::cout << router.matchPath("/users/admin/profile") << std::endl; // true
    std::cout << router.matchPath("/users/jane/settings") << std::endl; // false
    std::cout << router.matchPath("/posts/123") << std::endl;           // true

    return 0;

	 std::string route = "/users/jo*h*n*/profile";
    std::string path1 = "/users/john1/profile";
    std::string path2 = "/users/123/profile";
    std::string path3 = "/users/jane/settings";

    if (matchPath(route, path1)) {
        std::cout << "Path 1 matches the route." << std::endl;
    }

    if (matchPath(route, path2)) {
        std::cout << "Path 2 matches the route." << std::endl;
    }

    if (matchPath(route, path3)) {
        std::cout << "Path 3 matches the route." << std::endl;
    }

	return 0;
	int count = 0;
	bnet::Timer timer(g_context_);
	timer.start(std::chrono::seconds(5), [&]() {
		print_time();

		count++;
		if (count >= 3) {
			timer.stop();
		}
	});
//////////////////////////////tcp//////////////////////////////
#if 0
	// svr
    auto tcp_svr_ptr = std::make_shared<svr_proxy<tcp_svr>>(8);
    tcp_svr_ptr->start("0.0.0.0", "8888");
	tcp_svr_ptr->test_timer();
	//cli
	auto tcp_cli_ptr = std::make_shared<cli_proxy<tcp_cli>>(5);
	tcp_cli_ptr->start();
	for (int i = 0; i < 100; ++i) {
		tcp_cli_ptr->add("127.0.0.1", "8888");
	}
#endif
/////////////////////tcps////////////////////////////////////////////
#if defined(NET_USE_SSL)
	// svr
    auto tcps_svr_ptr = std::make_shared<svr_proxy<tcps_svr>>(4); 
	tcps_svr_ptr->set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);
	auto ec = tcps_svr_ptr->set_cert_buffer(ca_crt, server_crt, server_key, "123456");
	if (ec) {
		std::cout << "server:" << ec.message() << std::endl;
		return 0;
	}
	ec = tcps_svr_ptr->set_dh_buffer(dh);
	if (ec) {
		std::cout << "server:" << ec.message() << std::endl;
		return 0;
	}
    tcps_svr_ptr->start("0.0.0.0", "8888");
	tcps_svr_ptr->test_timer();
	//cli
	auto tcps_cli_ptr = std::make_shared<cli_proxy<tcps_cli>>(4);
	tcps_cli_ptr->set_verify_mode(asio::ssl::verify_peer);
	ec = tcps_cli_ptr->set_cert_buffer(ca_crt, client_crt, client_key, "123456");
	tcps_cli_ptr->start();
	if (ec) {
		std::cout << "server:" << ec.message() << std::endl;
		return 0;
	}
	for (int i = 0; i < 1; ++i) {
		tcps_cli_ptr->add("127.0.0.1", "8888");
	}
#endif
//////////////////////////////udp//////////////////////////////
#if 0
	// svr
    auto udp_svr_ptr = std::make_shared<svr_proxy<udp_svr>>(8);
    udp_svr_ptr->start("0.0.0.0", "8888");
	udp_svr_ptr->test_timer();
	//cli
	auto udp_cli_ptr = std::make_shared<cli_proxy<udp_cli>>(5);
	udp_cli_ptr->start();
	for (int i = 0; i < 100; ++i) {
		udp_cli_ptr->add("127.0.0.1", "8888");
	}
	//std::this_thread::sleep_for(std::chrono::seconds(2));
	//udp_cli_ptr->stop(bnet::ec_ignore);
#endif

	asio::signal_set signals(g_context_, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto){ g_context_.stop(); });

    auto io_worker = asio::make_work_guard(g_context_);
	g_context_.run();

    return 0;
}