/*
 * Copyright (c) 2023 bocai
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

namespace bnet::beast::http {
    /**
	 * The object wrapped for a url string like "http://www.github.com"
	 * <scheme>://<user>:<password>@<host>:<port>/<path>;<params>?<query>#<fragment>
	 */
	class url {
	public:
		/**
		 * @brief constructor
		 * if the 'str' has unencoded char, it will be encoded automatically, otherwise
		 * the url parsing will be failed.
		 */
		url(std::string str) : string_(std::move(str)) {
			std::memset((void*)(std::addressof(parser_)), 0, sizeof(http_parser_url));

			if (!string_.empty()) {
				bool has_unencode_ch = has_unencode_char(string_);

				if (has_unencode_ch) {
					string_ = url_encode(string_);
				}

				if (0 != http_parser_parse_url(string_.data(), string_.size(), 0, std::addressof(parser_))) {
					if (has_unencode_ch) {
						string_ = url_decode(string_);
					}

					set_last_error(asio::error::invalid_argument);
				}
			}
		}

		url(url&&) noexcept = default;
		url(url const&) = default;
		url& operator=(url&&) noexcept = default;
		url& operator=(url const&) = default;

		/**
		 * @brief Gets the content of the "schema" section, maybe empty
		 * The return value is usually http or https
		 */
		inline std::string_view get_schema() noexcept {
			return this->field(url_fields::UF_SCHEMA);
		}

		/**
		 * @brief Gets the content of the "schema" section, maybe empty
		 * The return value is usually http or https
		 * same as get_schema
		 */
		inline std::string_view schema() noexcept {
			return this->get_schema();
		}

		/**
		 * @brief Gets the content of the "host" section, maybe empty
		 */
		inline std::string_view get_host() noexcept {
			return this->field(url_fields::UF_HOST);
		}

		/**
		 * @brief Gets the content of the "host" section, maybe empty
		 * same as get_host
		 */
		inline std::string_view host() noexcept {
			return this->get_host();
		}

		inline std::string_view get_default_port() noexcept {
			std::string_view schema = this->schema();
			if (tool::str_equals(schema, "http"))
				return std::string_view{ "80" };
			if (tool::str_equals(schema, "https"))
				return std::string_view{ "443" };
			return std::string_view{ "80" };
		}

		inline std::string_view default_port() noexcept {
			return this->get_default_port();
		}

		/**
		 * @brief Gets the content of the "port" section
		 */
		inline std::string_view get_port() noexcept {
			std::string_view p = this->field(url_fields::UF_PORT);
			if (p.empty())
				return this->default_port();
			return p;
		}

		/**
		 * @brief Gets the content of the "port" section
		 * same as get_port
		 */
		inline std::string_view port() noexcept {
			return this->get_port();
		}

		/**
		 * @brief Gets the content of the "path" section
		 * the return value maybe has undecoded char, you can use http::url_decode(...) to decoded it.
		 */
		inline std::string_view get_path() noexcept {
			std::string_view p = this->field(url_fields::UF_PATH);
			if (p.empty())
				return std::string_view{ "/" };
			return p;
		}

		/**
		 * @brief Gets the content of the "path" section
		 * the return value maybe has undecoded char, you can use http::url_decode(...) to decoded it.
		 * same as get_path
		 */
		inline std::string_view path() noexcept {
			return this->get_path();
		}

		/**
		 * @brief Gets the content of the "query" section, maybe empty
		 * the return value maybe has undecoded char, you can use http::url_decode(...) to decoded it.
		 */
		inline std::string_view get_query() noexcept {
			return this->field(url_fields::UF_QUERY);
		}

		/**
		 * @brief Gets the content of the "query" section, maybe empty
		 * the return value maybe has undecoded char, you can use http::url_decode(...) to decoded it.
		 * same as get_query
		 */
		inline std::string_view query() noexcept {
			return this->get_query();
		}

		/**
		 * @brief Gets the "target", which composed by path and query
		 * the return value maybe has undecoded char, you can use http::url_decode(...) to decoded it.
		 */
		inline std::string_view get_target() noexcept {
			if (parser_.field_set & (1 << (int)url_fields::UF_PATH)) {
				return std::string_view{ &string_[
					parser_.field_data[(int)url_fields::UF_PATH].off],
					string_.size() -
					parser_.field_data[(int)url_fields::UF_PATH].off };
			}

			return std::string_view{ "/" };
		}

		/**
		 * @brief Gets the "target", which composed by path and query
		 * the return value maybe has undecoded char, you can use http::url_decode(...) to decoded it.
		 * same as get_target
		 */
		inline std::string_view target() noexcept {
			return this->get_target();
		}

		/**
		 * @brief Gets the content of the specific section, maybe empty
		 */
		inline std::string_view get_field(url_fields f) noexcept {
			if (!(parser_.field_set & (1 << int(f))))
				return std::string_view{};

			return std::string_view{ &string_[parser_.field_data[int(f)].off], parser_.field_data[int(f)].len };
		}

		/**
		 * @brief Gets the content of the specific section, maybe empty
		 * same as get_field
		 */
		inline std::string_view field(url_fields f) noexcept {
			return this->get_field(std::move(f));
		}

		inline http_parser_url &     parser() noexcept { return this->parser_; }
		inline std::string                   &     string() noexcept { return this->string_; }
		inline http_parser_url & get_parser() noexcept { return this->parser_; }
		inline std::string                   & get_string() noexcept { return this->string_; }

	protected:
		http_parser_url         parser_;
		std::string                           string_;
	};
}