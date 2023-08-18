#include <iostream> 
#include "http/router.hpp"

const static std::string routes[] = {
    "/",
	"/cmd/:tool/",
	"/cmd/:tool/:sub",
	"/cmd/whoami",
	"/cmd/whoami/root",
	"/cmd/whoami/root/",
	"/src/*filepath",
	"/search/",
	"/search/:query",
	"/search/gin-gonic",
	"/search/google",
	"/user_:name",
	"/user_:name/about",
	"/files/:dir/*filepath",
	"/doc/",
	"/doc/go_faq.html",
	"/doc/go1.html",
	"/info/:user/public",
	"/info/:user/project/:project",
	"/info/:user/project/golang",
	"/aa/*xx",
	"/ab/*xx",
	"/:cc",
	"/c1/:dd/e",
	"/c1/:dd/e1",
	"/:cc/cc",
	"/:cc/:dd/ee",
	"/:cc/:dd/:ee/ff",
	"/:cc/:dd/:ee/:ff/gg",
	"/:cc/:dd/:ee/:ff/:gg/hh",
	"/get/test/abc/",
	"/get/:param/abc/",
	"/something/:paramname/thirdthing",
	"/something/secondthing/test",
	"/get/abc",
	"/get/:param",
	"/get/abc/123abc",
	"/get/abc/:param",
	"/get/abc/123abc/xxx8",
	"/get/abc/123abc/:param",
	"/get/abc/123abc/xxx8/1234",
	"/get/abc/123abc/xxx8/:param",
	"/get/abc/123abc/xxx8/1234/ffas",
	"/get/abc/123abc/xxx8/1234/:param",
	"/get/abc/123abc/xxx8/1234/kkdd/12c",
	"/get/abc/123abc/xxx8/1234/kkdd/:param",
	"/get/abc/:param/test",
	"/get/abc/123abd/:param",
	"/get/abc/123abddd/:param",
	"/get/abc/123/:param",
	"/get/abc/123abg/:param",
	"/get/abc/123abf/:param",
	"/get/abc/123abfff/:param",
};

struct requests {
    std::string path = "";
    bool nilHandle = false;
    std::string route = "";
    std::vector<bnet::base::http::radix_tree::param> ps{};
};
int count = 0;
void check_request(bnet::base::http::radix_tree& tree, std::shared_ptr<bnet::base::http::node>& root, std::vector<requests> tstreq, bool unescapes = false) {
    for (auto &&req : tstreq) {
        std::vector<bnet::base::http::radix_tree::param> params;
		std::vector<bnet::base::http::radix_tree::skipped_node> skipped_nodes;
        auto value = tree.get_value(root, req.path, &params, skipped_nodes, unescapes);

        if (value.handlers_ == nullptr) {
            if (!req.nilHandle) {
                std::cerr << "handle mismatch for route " << req.path << " : Expected non-ps handle" << std::endl;
            } 
        }
        else if (req.nilHandle) {
            std::cerr << "handle mismatch for route " << req.path << "Expected ps handle 1" << std::endl;
        }
        else {
            auto ret = value.handlers_();
			count++;
			std::cout << ret << " : " << req.route << ", " << count << std::endl;
            if (ret != req.route) {
                std::cerr << "handle mismatch for route " << req.path << ", " << req.route << ", " << ret << std::endl;
            }
        }
        if (value.params_ != nullptr) {
            if (value.params_->size() != req.ps.size()) {
                std::cerr << "Params mismatch for route " << req.path << std::endl;

            }
            //auto ps = req.ps;
        }
    }
}

uint32_t check_priorities(std::shared_ptr<bnet::base::http::node>& node) {
    uint32_t prio = 0;
    for (auto &&i : node->children_) {
        prio += check_priorities(i);
    }

    if (node->handlers_ != nullptr) {
        prio++;
    }

    if (node->priority_ != prio) {
        std::cerr << "priority mismatch for node " << node->path_ << ", " << node->priority_ << ", " << prio << std::endl;
    }

    return prio;
}

void test_rount () {
    bnet::base::http::radix_tree tree;
	auto root = std::make_shared<bnet::base::http::node>();

    for (auto &&i : routes) {
        tree.add_route(root, i, [&](){ return i;});    
    }
    
    std::vector<bnet::base::http::radix_tree::param> ps{};
    check_request(tree, root, std::vector<requests>{
		{"/", false, "/", ps},
		{"/cmd/test", true, "/cmd/:tool/", std::vector<bnet::base::http::radix_tree::param>{{"tool", "test"}}},
		{"/cmd/test/", false, "/cmd/:tool/", std::vector<bnet::base::http::radix_tree::param>{{"tool", "test"}}},
		{"/cmd/test/3", false, "/cmd/:tool/:sub", std::vector<bnet::base::http::radix_tree::param>{{"tool", "test"}, {"sub", "3"}}},
		{"/cmd/who", true, "/cmd/:tool/", std::vector<bnet::base::http::radix_tree::param>{{"tool", "who"}}},
		{"/cmd/who/", false, "/cmd/:tool/", std::vector<bnet::base::http::radix_tree::param>{{"tool", "who"}}},
		{"/cmd/whoami", false, "/cmd/whoami", ps},
		{"/cmd/whoami/", true, "/cmd/whoami", ps},
		{"/cmd/whoami/r", false, "/cmd/:tool/:sub", std::vector<bnet::base::http::radix_tree::param>{{"tool", "whoami"}, {"sub", "r"}}},
		{"/cmd/whoami/r/", true, "/cmd/:tool/:sub", std::vector<bnet::base::http::radix_tree::param>{{"tool", "whoami"}, {"sub", "r"}}},
		{"/cmd/whoami/root", false, "/cmd/whoami/root", ps},
		{"/cmd/whoami/root/", false, "/cmd/whoami/root/", ps},
		{"/src/", false, "/src/*filepath", std::vector<bnet::base::http::radix_tree::param>{{"filepath", "/"}}},
		{"/src/some/file.png", false, "/src/*filepath", std::vector<bnet::base::http::radix_tree::param>{{"filepath", "/some/file.png"}}},
		{"/search/", false, "/search/", ps},
		{"/search/someth!ng+in+ünìcodé", false, "/search/:query", std::vector<bnet::base::http::radix_tree::param>{{"query", "someth!ng+in+ünìcodé"}}},
		{"/search/someth!ng+in+ünìcodé/", true, "", std::vector<bnet::base::http::radix_tree::param>{{"query", "someth!ng+in+ünìcodé"}}},
		{"/search/gin", false, "/search/:query", std::vector<bnet::base::http::radix_tree::param>{{"query", "gin"}}},
		{"/search/gin-gonic", false, "/search/gin-gonic", ps},
		{"/search/google", false, "/search/google", ps},
		{"/user_gopher", false, "/user_:name", std::vector<bnet::base::http::radix_tree::param>{{"name", "gopher"}}},
		{"/user_gopher/about", false, "/user_:name/about", std::vector<bnet::base::http::radix_tree::param>{{"name", "gopher"}}},
		{"/files/js/inc/framework.js", false, "/files/:dir/*filepath", std::vector<bnet::base::http::radix_tree::param>{{"dir", "js"}, {"filepath", "/inc/framework.js"}}},
		{"/info/gordon/public", false, "/info/:user/public", std::vector<bnet::base::http::radix_tree::param>{{"user", "gordon"}}},
		{"/info/gordon/project/go", false, "/info/:user/project/:project", std::vector<bnet::base::http::radix_tree::param>{{"user", "gordon"}, {"project", "go"}}},
		{"/info/gordon/project/golang", false, "/info/:user/project/golang", std::vector<bnet::base::http::radix_tree::param>{{"user", "gordon"}}},
		{"/aa/aa", false, "/aa/*xx", std::vector<bnet::base::http::radix_tree::param>{{"xx", "/aa"}}},
		{"/ab/ab", false, "/ab/*xx", std::vector<bnet::base::http::radix_tree::param>{{"xx", "/ab"}}},
		{"/a", false, "/:cc", std::vector<bnet::base::http::radix_tree::param>{{"cc", "a"}}},
		// * Error with argument being intercepted
		// new PR handle (/all /all/cc /a/cc)
		// fix PR: https://github.com/gin-gonic/gin/pull/2796
		{"/all", false, "/:cc", std::vector<bnet::base::http::radix_tree::param>{{"cc", "all"}}},
		{"/d", false, "/:cc", std::vector<bnet::base::http::radix_tree::param>{{"cc", "d"}}},
		{"/ad", false, "/:cc", std::vector<bnet::base::http::radix_tree::param>{{"cc", "ad"}}},
		{"/dd", false, "/:cc", std::vector<bnet::base::http::radix_tree::param>{{"cc", "dd"}}},
		{"/dddaa", false, "/:cc", std::vector<bnet::base::http::radix_tree::param>{{"cc", "dddaa"}}},
		{"/aa", false, "/:cc", std::vector<bnet::base::http::radix_tree::param>{{"cc", "aa"}}},
		{"/aaa", false, "/:cc", std::vector<bnet::base::http::radix_tree::param>{{"cc", "aaa"}}},
		{"/aaa/cc", false, "/:cc/cc", std::vector<bnet::base::http::radix_tree::param>{{"cc", "aaa"}}},
		{"/ab", false, "/:cc", std::vector<bnet::base::http::radix_tree::param>{{"cc", "ab"}}},
		{"/abb", false, "/:cc", std::vector<bnet::base::http::radix_tree::param>{{"cc", "abb"}}},
		{"/abb/cc", false, "/:cc/cc", std::vector<bnet::base::http::radix_tree::param>{{"cc", "abb"}}},
		{"/allxxxx", false, "/:cc", std::vector<bnet::base::http::radix_tree::param>{{"cc", "allxxxx"}}},
		{"/alldd", false, "/:cc", std::vector<bnet::base::http::radix_tree::param>{{"cc", "alldd"}}},
		{"/all/cc", false, "/:cc/cc", std::vector<bnet::base::http::radix_tree::param>{{"cc", "all"}}},
		{"/a/cc", false, "/:cc/cc", std::vector<bnet::base::http::radix_tree::param>{{"cc", "a"}}},
		{"/c1/d/e", false, "/c1/:dd/e", std::vector<bnet::base::http::radix_tree::param>{{"dd", "d"}}},
		{"/c1/d/e1", false, "/c1/:dd/e1", std::vector<bnet::base::http::radix_tree::param>{{"dd", "d"}}},
		{"/c1/d/ee", false, "/:cc/:dd/ee", std::vector<bnet::base::http::radix_tree::param>{{"cc", "c1"}, {"dd", "d"}}},
		{"/cc/cc", false, "/:cc/cc", std::vector<bnet::base::http::radix_tree::param>{{"cc", "cc"}}},
		{"/ccc/cc", false, "/:cc/cc", std::vector<bnet::base::http::radix_tree::param>{{"cc", "ccc"}}},
		{"/deedwjfs/cc", false, "/:cc/cc", std::vector<bnet::base::http::radix_tree::param>{{"cc", "deedwjfs"}}},
		{"/acllcc/cc", false, "/:cc/cc", std::vector<bnet::base::http::radix_tree::param>{{"cc", "acllcc"}}},
		{"/get/test/abc/", false, "/get/test/abc/", ps},
		{"/get/te/abc/", false, "/get/:param/abc/", std::vector<bnet::base::http::radix_tree::param>{{"param", "te"}}},
		{"/get/testaa/abc/", false, "/get/:param/abc/", std::vector<bnet::base::http::radix_tree::param>{{"param", "testaa"}}},
		{"/get/xx/abc/", false, "/get/:param/abc/", std::vector<bnet::base::http::radix_tree::param>{{"param", "xx"}}},
		{"/get/tt/abc/", false, "/get/:param/abc/", std::vector<bnet::base::http::radix_tree::param>{{"param", "tt"}}},
		{"/get/a/abc/", false, "/get/:param/abc/", std::vector<bnet::base::http::radix_tree::param>{{"param", "a"}}},
		{"/get/t/abc/", false, "/get/:param/abc/", std::vector<bnet::base::http::radix_tree::param>{{"param", "t"}}},
		{"/get/aa/abc/", false, "/get/:param/abc/", std::vector<bnet::base::http::radix_tree::param>{{"param", "aa"}}},
		{"/get/abas/abc/", false, "/get/:param/abc/", std::vector<bnet::base::http::radix_tree::param>{{"param", "abas"}}},
		{"/something/secondthing/test", false, "/something/secondthing/test", ps},
		{"/something/abcdad/thirdthing", false, "/something/:paramname/thirdthing", std::vector<bnet::base::http::radix_tree::param>{{"paramname", "abcdad"}}},
		{"/something/secondthingaaaa/thirdthing", false, "/something/:paramname/thirdthing", std::vector<bnet::base::http::radix_tree::param>{{"paramname", "secondthingaaaa"}}},
		{"/something/se/thirdthing", false, "/something/:paramname/thirdthing", std::vector<bnet::base::http::radix_tree::param>{{"paramname", "se"}}},
		{"/something/s/thirdthing", false, "/something/:paramname/thirdthing", std::vector<bnet::base::http::radix_tree::param>{{"paramname", "s"}}},
		{"/c/d/ee", false, "/:cc/:dd/ee", std::vector<bnet::base::http::radix_tree::param>{{"cc", "c"}, {"dd", "d"}}},
		{"/c/d/e/ff", false, "/:cc/:dd/:ee/ff", std::vector<bnet::base::http::radix_tree::param>{{"cc", "c"}, {"dd", "d"}, {"ee", "e"}}},
		{"/c/d/e/f/gg", false, "/:cc/:dd/:ee/:ff/gg", std::vector<bnet::base::http::radix_tree::param>{{"cc", "c"}, {"dd", "d"}, {"ee", "e"}, {"ff", "f"}}},
		{"/c/d/e/f/g/hh", false, "/:cc/:dd/:ee/:ff/:gg/hh", std::vector<bnet::base::http::radix_tree::param>{{"cc", "c"}, {"dd", "d"}, {"ee", "e"}, {"ff", "f"}, {"gg", "g"}}},
		{"/cc/dd/ee/ff/gg/hh", false, "/:cc/:dd/:ee/:ff/:gg/hh", std::vector<bnet::base::http::radix_tree::param>{{"cc", "cc"}, {"dd", "dd"}, {"ee", "ee"}, {"ff", "ff"}, {"gg", "gg"}}},
		{"/get/abc", false, "/get/abc", ps},
		{"/get/a", false, "/get/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "a"}}},
		{"/get/abz", false, "/get/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "abz"}}},
		{"/get/12a", false, "/get/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "12a"}}},
		{"/get/abcd", false, "/get/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "abcd"}}},
		{"/get/abc/123abc", false, "/get/abc/123abc", ps},
		{"/get/abc/12", false, "/get/abc/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "12"}}},
		{"/get/abc/123ab", false, "/get/abc/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "123ab"}}},
		{"/get/abc/xyz", false, "/get/abc/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "xyz"}}},
		{"/get/abc/123abcddxx", false, "/get/abc/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "123abcddxx"}}},
		{"/get/abc/123abc/xxx8", false, "/get/abc/123abc/xxx8", ps},
		{"/get/abc/123abc/x", false, "/get/abc/123abc/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "x"}}},
		{"/get/abc/123abc/xxx", false, "/get/abc/123abc/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "xxx"}}},
		{"/get/abc/123abc/abc", false, "/get/abc/123abc/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "abc"}}},
		{"/get/abc/123abc/xxx8xxas", false, "/get/abc/123abc/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "xxx8xxas"}}},
		{"/get/abc/123abc/xxx8/1234", false, "/get/abc/123abc/xxx8/1234", ps},
		{"/get/abc/123abc/xxx8/1", false, "/get/abc/123abc/xxx8/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "1"}}},
		{"/get/abc/123abc/xxx8/123", false, "/get/abc/123abc/xxx8/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "123"}}},
		{"/get/abc/123abc/xxx8/78k", false, "/get/abc/123abc/xxx8/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "78k"}}},
		{"/get/abc/123abc/xxx8/1234xxxd", false, "/get/abc/123abc/xxx8/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "1234xxxd"}}},
		{"/get/abc/123abc/xxx8/1234/ffas", false, "/get/abc/123abc/xxx8/1234/ffas", ps},
		{"/get/abc/123abc/xxx8/1234/f", false, "/get/abc/123abc/xxx8/1234/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "f"}}},
		{"/get/abc/123abc/xxx8/1234/ffa", false, "/get/abc/123abc/xxx8/1234/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "ffa"}}},
		{"/get/abc/123abc/xxx8/1234/kka", false, "/get/abc/123abc/xxx8/1234/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "kka"}}},
		{"/get/abc/123abc/xxx8/1234/ffas321", false, "/get/abc/123abc/xxx8/1234/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "ffas321"}}},
		{"/get/abc/123abc/xxx8/1234/kkdd/12c", false, "/get/abc/123abc/xxx8/1234/kkdd/12c", ps},
		{"/get/abc/123abc/xxx8/1234/kkdd/1", false, "/get/abc/123abc/xxx8/1234/kkdd/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "1"}}},
		{"/get/abc/123abc/xxx8/1234/kkdd/12", false, "/get/abc/123abc/xxx8/1234/kkdd/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "12"}}},
		{"/get/abc/123abc/xxx8/1234/kkdd/12b", false, "/get/abc/123abc/xxx8/1234/kkdd/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "12b"}}},
		{"/get/abc/123abc/xxx8/1234/kkdd/34", false, "/get/abc/123abc/xxx8/1234/kkdd/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "34"}}},
		{"/get/abc/123abc/xxx8/1234/kkdd/12c2e3", false, "/get/abc/123abc/xxx8/1234/kkdd/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "12c2e3"}}},
		{"/get/abc/12/test", false, "/get/abc/:param/test", std::vector<bnet::base::http::radix_tree::param>{{"param", "12"}}},
		{"/get/abc/123abdd/test", false, "/get/abc/:param/test", std::vector<bnet::base::http::radix_tree::param>{{"param", "123abdd"}}},
		{"/get/abc/123abdddf/test", false, "/get/abc/:param/test", std::vector<bnet::base::http::radix_tree::param>{{"param", "123abdddf"}}},
		{"/get/abc/123ab/test", false, "/get/abc/:param/test", std::vector<bnet::base::http::radix_tree::param>{{"param", "123ab"}}},
		{"/get/abc/123abgg/test", false, "/get/abc/:param/test", std::vector<bnet::base::http::radix_tree::param>{{"param", "123abgg"}}},
		{"/get/abc/123abff/test", false, "/get/abc/:param/test", std::vector<bnet::base::http::radix_tree::param>{{"param", "123abff"}}},
		{"/get/abc/123abffff/test", false, "/get/abc/:param/test", std::vector<bnet::base::http::radix_tree::param>{{"param", "123abffff"}}},
		{"/get/abc/123abd/test", false, "/get/abc/123abd/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "test"}}},
		{"/get/abc/123abddd/test", false, "/get/abc/123abddd/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "test"}}},
		{"/get/abc/123/test22", false, "/get/abc/123/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "test22"}}},
		{"/get/abc/123abg/test", false, "/get/abc/123abg/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "test"}}},
		{"/get/abc/123abf/testss", false, "/get/abc/123abf/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "testss"}}},
		{"/get/abc/123abfff/te", false, "/get/abc/123abfff/:param", std::vector<bnet::base::http::radix_tree::param>{{"param", "te"}}},
	});

	check_priorities(root);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
/*
struct tst {
	std::string in = "";
	std::string out = "";
	bool found = false;
	bool slash = false;
};
void tst_tree() {
	std::string ostr('o', 128);
	std::string Ostr('O', 128);
	std::string longpath = "/l" + ostr + "ng";
	std::string lOngpath = "/l" + Ostr + "ng/";
	const static std::string routes_tst[] = {
		"/hi",
		"/b/",
		"/ABC/",
		"/search/:query",
		"/cmd/:tool/",
		"/src/*filepath",
		"/x",
		"/x/y",
		"/y/",
		"/y/z",
		"/0/:id",
		"/0/:id/1",
		"/1/:id/",
		"/1/:id/2",
		"/aa",
		"/a/",
		"/doc",
		"/doc/go_faq.html",
		"/doc/go1.html",
		"/doc/go/away",
		"/no/a",
		"/no/b",
		"/Π",
		"/u/apfêl/",
		"/u/äpfêl/",
		"/u/öpfêl",
		"/v/Äpfêl/",
		"/v/Öpfêl",
		"/w/♬",  // 3 byte
		"/w/♭/", // 3 byte, last byte differs
		"/w/𠜎",  // 4 byte
		"/w/𠜏/", // 4 byte
		longpath,
	};

	bnet::base::http::radix_tree tree;
	auto root = std::make_shared<bnet::base::http::node>();

    for (auto &&i : routes_tst) {
        tree.add_route(root, i, [&](){ return i;});    
    }

	for (auto &&i : routes_tst) {
		auto ret = tree.finc_case_insensitive_path(root, i, true);
		if (ret == nullptr) {
			std::cerr << "Route not found! rount:" << i  << std::endl;
		}
		if (*ret != i) {
			std::cerr << "Wrong result for route " << i << " : " << *ret << std::endl;
		}
	}
	
	for (auto &&i : routes_tst) {
		auto ret = tree.finc_case_insensitive_path(root, i, false);
		if (ret == nullptr) {
			std::cerr << "Route not found! rount:" << i  << std::endl;
		}
		if (*ret != i) {
			std::cerr << "Wrong result for route " << i << " : " << *ret << std::endl;
		}
	}

	const static tst tests[] = {
		{"/HI", "/hi", true, false},
		{"/HI/", "/hi", true, true},
		{"/B", "/b/", true, true},
		{"/B/", "/b/", true, false},
		{"/abc", "/ABC/", true, true},
		{"/abc/", "/ABC/", true, false},
		{"/aBc", "/ABC/", true, true},
		{"/aBc/", "/ABC/", true, false},
		{"/abC", "/ABC/", true, true},
		{"/abC/", "/ABC/", true, false},
		{"/SEARCH/QUERY", "/search/QUERY", true, false},
		{"/SEARCH/QUERY/", "/search/QUERY", true, true},
		{"/CMD/TOOL/", "/cmd/TOOL/", true, false},
		{"/CMD/TOOL", "/cmd/TOOL/", true, true},
		{"/SRC/FILE/PATH", "/src/FILE/PATH", true, false},
		{"/x/Y", "/x/y", true, false},
		{"/x/Y/", "/x/y", true, true},
		{"/X/y", "/x/y", true, false},
		{"/X/y/", "/x/y", true, true},
		{"/X/Y", "/x/y", true, false},
		{"/X/Y/", "/x/y", true, true},
		{"/Y/", "/y/", true, false},
		{"/Y", "/y/", true, true},
		{"/Y/z", "/y/z", true, false},
		{"/Y/z/", "/y/z", true, true},
		{"/Y/Z", "/y/z", true, false},
		{"/Y/Z/", "/y/z", true, true},
		{"/y/Z", "/y/z", true, false},
		{"/y/Z/", "/y/z", true, true},
		{"/Aa", "/aa", true, false},
		{"/Aa/", "/aa", true, true},
		{"/AA", "/aa", true, false},
		{"/AA/", "/aa", true, true},
		{"/aA", "/aa", true, false},
		{"/aA/", "/aa", true, true},
		{"/A/", "/a/", true, false},
		{"/A", "/a/", true, true},
		{"/DOC", "/doc", true, false},
		{"/DOC/", "/doc", true, true},
		{"/NO", "", false, true},
		{"/DOC/GO", "", false, true},
		{"/π", "/Π", true, false},
		{"/π/", "/Π", true, true},
		{"/u/ÄPFÊL/", "/u/äpfêl/", true, false},
		{"/u/ÄPFÊL", "/u/äpfêl/", true, true},
		{"/u/ÖPFÊL/", "/u/öpfêl", true, true},
		{"/u/ÖPFÊL", "/u/öpfêl", true, false},
		{"/v/äpfêL/", "/v/Äpfêl/", true, false},
		{"/v/äpfêL", "/v/Äpfêl/", true, true},
		{"/v/öpfêL/", "/v/Öpfêl", true, true},
		{"/v/öpfêL", "/v/Öpfêl", true, false},
		{"/w/♬/", "/w/♬", true, true},
		{"/w/♭", "/w/♭/", true, true},
		{"/w/𠜎/", "/w/𠜎", true, true},
		{"/w/𠜏", "/w/𠜏/", true, true},
		{lOngpath, longpath, true, true},
	};

	for (auto &&test : tests) {
		auto ret = tree.finc_case_insensitive_path(root, test.in, true);
		if (ret == nullptr) {
			std::cerr << "Route not found! rount:" << test.in << std::endl;
		}
		if (*ret != test.out) {
			std::cerr << "Wrong result for " << test.in << " : get" << *ret << " ; want " << test.out << std::endl;
		}
	}

	for (auto &&test : tests) {
		auto ret = tree.finc_case_insensitive_path(root, test.in, false);
		if (ret == nullptr) {
			std::cerr << "Route not found! rount:" << test.in << std::endl;
		}
		if (*ret != test.out) {
			std::cerr << "Wrong result for " << test.in << " : get" << *ret << " ; want " << test.out << std::endl;
		}
	}
}
*/