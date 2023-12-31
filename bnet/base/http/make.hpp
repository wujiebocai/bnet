/*
 * Copyright (c) 2023 bocai
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

namespace bnet::beast::http {
	/**
	 * @brief make A typical HTTP request struct from the uri
	 * if the string is a url, it must start with http or https,
	 * eg : the url must be "https://www.github.com", the url can't be "www.github.com"
	 */
	template<class String>
	web_request make_request(String&& uri) {
		web_request req;

		clear_last_error();

		std::string surl = tool::to_string(std::forward<String>(uri));

		// if uri is empty, break
		if (surl.empty()) {
			set_last_error(::asio::error::invalid_argument);
			return req;
		}

		bool has_crlf = (surl.find("\r\n") != std::string::npos);

		std::string buf = (has_crlf ? surl : "");

		req.url() = url{ std::move(surl) };

		// If a \r\n string is found, it is not a URL
		// If get_last_error is not zero, it means that the uri is not a URL.
		if (has_crlf && get_last_error()) {
			http::request_parser<http::string_body> parser;
			parser.eager(true);
			parser.put(::asio::buffer(buf), get_last_error());
			req = parser.get();
		}
		// It is a URL
		else {
			if (get_last_error())
				return req;

			/* <scheme>://<user>:<password>@<host>:<port>/<path>;<params>?<query>#<fragment> */

			std::string_view host   = req.url().host();
			std::string_view port   = req.url().port();
			std::string_view target = req.url().target();

			// Set up an HTTP GET request message
			req.method(http::verb::get);
			req.version(11);
			req.target(string_view(target.data(), target.size()));
			//req.set(http::field::server, BEAST_VERSION_STRING);

			if (host.empty()) {
				set_last_error(::asio::error::invalid_argument);
				return req;
			}

			if (!port.empty() && port != "443" && port != "80")
				req.set(http::field::host, std::string(host) + ":" + std::string(port));
			else
				req.set(http::field::host, host);
		}

		return req;
	}

	/**
	 * @brief make A typical HTTP request struct from the uri
	 */
	template<typename String, typename StrOrInt>
	inline web_request make_request(String&& host, StrOrInt&& port,
		std::string_view target, http::verb method = http::verb::get, unsigned version = 11) {
		web_request req;

		clear_last_error();

		std::string h = tool::to_string(std::forward<String>(host));
		tool::trim_both(h);

		std::string p = tool::to_string(std::forward<StrOrInt>(port));
		tool::trim_both(p);

		std::string_view schema{ h.data(), (std::min<std::string_view::size_type>)(4, h.size()) };

		std::string surl;
		if (!tool::str_equals(schema, "http")) {
			if /**/ (p == "80")
				surl += "http://";
			else if (p == "443")
				surl += "https://";
			else
				surl += "http://";
		}
		surl += h;
		if (!p.empty() && p != "443" && p != "80") {
			surl += ":";
			surl += p;
		}
		surl += target;

		req.url() = url{ std::move(surl) };

		// if url is invalid, break
		if (get_last_error())
			return req;

		// Set up an HTTP GET request message
		req.method(method);
		req.version(version);
		if (target.empty()) {
			req.target(string_view{ "/" });
		}
		else {
			if (has_unencode_char(target, 1)) {
				std::string encoded = url_encode(target);
				req.target(string_view(encoded.data(), encoded.size()));
			}
			else {
				req.target(string_view(target.data(), target.size()));
			}
		}
		if (!p.empty() && p != "443" && p != "80") {
			req.set(http::field::host, h + ":" + p);
		}
		else {
			req.set(http::field::host, std::move(h));
		}
		//req.set(http::field::server, BEAST_VERSION_STRING);

		return req;
	}

	template<class Body = http::string_body, class Fields = http::fields>
	inline http::response<Body, Fields> make_response(std::string_view uri) {
		clear_last_error();
		http::response_parser<Body> parser;
		parser.eager(true);
		parser.put(::asio::buffer(uri), get_last_error());
		http::response<Body, Fields> rep = parser.get();
		return rep;
	}

	template<class Body = http::string_body, class Fields = http::fields>
	inline typename std::enable_if_t<std::is_same_v<Body, http::string_body>, http::response<Body, Fields>>
		make_response(http::status code, std::string_view body, unsigned version = 11) {
		http::response<Body, Fields> rep;
		rep.version(version);
		//rep.set(http::field::server, BEAST_VERSION_STRING);
		rep.result(code);
		rep.body() = body;
		try_prepare_payload(rep);
		return rep;
	}

	template<class Body = http::string_body, class Fields = http::fields>
	inline typename std::enable_if_t<std::is_same_v<Body, http::string_body>, http::response<Body, Fields>>
		make_response(unsigned code, std::string_view body, unsigned version = 11) {
		return make_response(http::int_to_status(code), body, version);
	}
}
