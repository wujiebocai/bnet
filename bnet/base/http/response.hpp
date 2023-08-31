#pragma once

// 待优化，basic_file_body.hpp里面344的断言。
namespace bnet::beast::http {
	class response_defer {
	public:
		response_defer(std::function<void()> cb, std::shared_ptr<void> session)
			: cb_(std::move(cb)), session_(std::move(session)) {
			NET_ASSERT(session_);
		}
		~response_defer() {
			if (cb_) { cb_(); }
		}

	protected:
		std::function<void()> cb_;

		// hold the http_session ptr, otherwise when the cb_ is calling, the session
		// maybe destroyed already, then the response("rep_") in the cb_ is destroyed
		// already, then it cause crash.
		std::shared_ptr<void> session_;
	};
}

namespace bnet::beast::http {
	template<class Body, class Fields = http::fields>
	class http_response_impl_t
		: public http::message<false, Body, Fields>
	{
	public:
		using self  = http_response_impl_t<Body, Fields>;
		using super = http::message<false, Body, Fields>;
		using header_type = typename super::header_type;
		using body_type = typename super::body_type;

	public:
		/**
		 * @brief constructor
		 * this default constructor it used for rdc call, beacuse the default status of
		 * http response is 200, and if the rdc call failed, the status of the returned
		 * http response is 200 too, so we set the status to another value at here.
		 */
		explicit http_response_impl_t()
			: super()
		{
			super::result(http::status::unknown);
		}

		template<typename... Args>
		explicit http_response_impl_t(Args&&... args)
			: super(std::forward<Args>(args)...)
		{
		}

		http_response_impl_t(const http_response_impl_t& o)
			: super(o.base())
		{
			//this->base() = o.base();
			this->root_directory_ = o.root_directory_;
			this->defer_callback_ = o.defer_callback_;
			this->refresh_func_ = o.refresh_func_;
			this->defer_guard_    = o.defer_guard_;
			this->session_ptr_    = o.session_ptr_;
		}

		http_response_impl_t(http_response_impl_t&& o)
			: super()
		{
			this->base() = std::move(o.base());
			this->root_directory_ = o.root_directory_;
			this->defer_callback_ = o.defer_callback_;
			this->refresh_func_ = o.refresh_func_;
			this->defer_guard_    = o.defer_guard_;
			this->session_ptr_    = o.session_ptr_;
		}

		self& operator=(const http_response_impl_t& o)
		{
			this->base() = o.base();
			this->root_directory_ = o.root_directory_;
			this->defer_callback_ = o.defer_callback_;
			this->refresh_func_ = o.refresh_func_;
			this->defer_guard_    = o.defer_guard_;
			this->session_ptr_    = o.session_ptr_;
			return *this;
		}

		self& operator=(http_response_impl_t&& o)
		{
			this->base() = std::move(o.base());
			this->root_directory_ = o.root_directory_;
			this->defer_callback_ = o.defer_callback_;
			this->refresh_func_ = o.refresh_func_;
			this->defer_guard_    = o.defer_guard_;
			this->session_ptr_    = o.session_ptr_;
			return *this;
		}

		template<class BodyT = Body>
		http_response_impl_t(const http::message<false, BodyT, Fields>& rep)
			: super()
		{
			this->base() = rep;
		}

		template<class BodyT = Body>
		http_response_impl_t(http::message<false, BodyT, Fields>&& rep)
			: super()
		{
			this->base() = std::move(rep);
		}

		template<class BodyT = Body>
		self& operator=(const http::message<false, BodyT, Fields>& rep) {
			this->base() = rep;
			return *this;
		}

		template<class BodyT = Body>
		self& operator=(http::message<false, BodyT, Fields>&& rep) {
			this->base() = std::move(rep);
			return *this;
		}

		//-------------------------------------------------

		http_response_impl_t(const http::message<false, http::string_body, Fields>& rep)
			: super()
		{
			this->base().base() = rep.base();
			this->body().text() = rep.body();
			try_prepare_payload(*this);
		}

		http_response_impl_t(http::message<false, http::string_body, Fields>&& rep)
			: super()
		{
			this->base().base() = std::move(rep.base());
			this->body().text() = std::move(rep.body());
			try_prepare_payload(*this);
		}

		self& operator=(const http::message<false, http::string_body, Fields>& rep) {
			this->base().base() = rep.base();
			this->body().text() = rep.body();
			try_prepare_payload(*this);
			return *this;
		}

		self& operator=(http::message<false, http::string_body, Fields>&& rep) {
			this->base().base() = std::move(rep.base());
			this->body().text() = std::move(rep.body());
			try_prepare_payload(*this);
			return *this;
		}

		//-------------------------------------------------

		http_response_impl_t(const http::message<false, http::file_body, Fields>& rep)
			: super()
		{
			this->base().base() = rep.base();
			this->body().file() = rep.body();
			try_prepare_payload(*this);
		}

		http_response_impl_t(http::message<false, http::file_body, Fields>&& rep)
			: super()
		{
			this->base().base() = std::move(rep.base());
			this->body().file() = std::move(rep.body());
			try_prepare_payload(*this);
		}

		self& operator=(const http::message<false, http::file_body, Fields>& rep) {
			this->base().base() = rep.base();
			this->body().file() = rep.body();
			try_prepare_payload(*this);
			return *this;
		}

		self& operator=(http::message<false, http::file_body, Fields>&& rep) {
			this->base().base() = std::move(rep.base());
			this->body().file() = std::move(rep.body());
			try_prepare_payload(*this);
			return *this;
		}

		/**
		 * @brief destructor
		 */
		~http_response_impl_t() {}

		/// Returns the base portion of the message
		inline super const& base() const noexcept {
			return *this;
		}

		/// Returns the base portion of the message
		inline super& base() noexcept {
			return *this;
		}

		inline void reset() {
			static_cast<super&>(*this) = {};

			this->result(http::status::unknown);
		}

		/**
		 * @brief set the root directory where we load the files.
		 */
		inline self& set_root_directory(std::filesystem::path path) {
			this->root_directory_ = std::move(path);
			return *this;
		}

		/**
		 * @brief get the root directory where we load the files.
		 */
		inline const std::filesystem::path& get_root_directory() noexcept {
			return this->root_directory_;
		}

		/**
		 * @brief create a deferred http response, the response will not be send immediately,
		 * the http response will be sent only when the returned std::shared_ptr<http::response_defer>
		 * is completely destroyed
		 */
		inline std::shared_ptr<response_defer> defer() {
			this->defer_guard_ = std::make_shared<response_defer>(
				this->defer_callback_, this->session_ptr_.lock());
			return this->defer_guard_;
		}

	public:
		/**
		 * @brief Respond to http request with plain text content
		 * @param content - the response body, it's usually a simple string,
		 * and the content-type is "text/plain" by default.
		 */
		template<class StringT>
		inline self& fill_text(StringT&& content, http::status result = http::status::ok,
			std::string_view mimetype = "text/plain", unsigned version = 11) {
			// must clear file_body
			this->body().file().close();

			this->set(http::field::server, BEAST_VERSION_STRING);
			this->set(http::field::content_type, mimetype.empty() ? "text/plain" : mimetype);

			this->result(result);
			this->version(version < 10 ? 11 : version);

			this->body().text() = tool::to_string(std::forward<StringT>(content));
			try_prepare_payload(*this);

			return (*this);
		}

		/**
		 * @brief Respond to http request with json content
		 */
		template<class StringT>
		inline self& fill_json(StringT&& content, http::status result = http::status::ok,
			std::string_view mimetype = "application/json", unsigned version = 11) {
			return this->fill_text(std::forward<StringT>(content), result,
				mimetype.empty() ? "application/json" : mimetype, version);
		}

		/**
		 * @brief Respond to http request with html content
		 * @param content - the response body, may be a plain text string, or a stardand
		 * <html>...</html> string, it's just that the content-type is "text/html" by default.
		 */
		template<class StringT>
		inline self& fill_html(StringT&& content, http::status result = http::status::ok,
			std::string_view mimetype = "text/html", unsigned version = 11) {
			return this->fill_text(std::forward<StringT>(content), result,
				mimetype.empty() ? "text/html" : mimetype, version);
		}

		/**
		 * @brief Respond to http request with pre-prepared error page content
		 * Generated a standard html error page automatically use the status coe 'result',
		 * like <html>...</html>, and the content-type is "text/html" by default.
		 */
		template<class StringT = std::string_view>
		inline self& fill_page(http::status result, StringT&& desc = std::string_view{},
			std::string_view mimetype = "text/html", unsigned version = 11) {
			return this->fill_text(error_page(result, std::forward<StringT>(desc)), result,
				mimetype.empty() ? "text/html" : mimetype, version);
		}

		/**
		 * @brief Respond to http request with local file
		 */
		inline self& fill_file(std::filesystem::path path,
			http::status result = http::status::ok, unsigned version = 11) {
			// if you want to build a absolute path by youself and passed it to fill_file function,
			// call set_root_directory("") first, then passed you absolute path to fill_file is ok.

			// Build the path to the requested file
			std::filesystem::path filepath;
			if (this->root_directory_.empty()) {
				filepath = std::move(path);
				filepath.make_preferred();
			}
			else {
				filepath = this->root_directory_;
				filepath.make_preferred();
				filepath /= path.make_preferred().relative_path();
			}

			// Attempt to open the file
			error_code ec;
			this->body().file().open(filepath.string().c_str(), file_mode::scan, ec);

			// Handle the case where the file doesn't exist
			if (ec == errc::no_such_file_or_directory)
				return this->fill_page(http::status::not_found, {}, {}, version);

			// Handle an unknown error
			if (ec)
				return this->fill_page(http::status::not_found, ec.message(), {}, version);

			// Cache the size since we need it after the move
			auto const size = this->body().size();

			// Respond to GET request
			this->content_length(size);

			this->set(http::field::server, BEAST_VERSION_STRING);
			this->set(http::field::content_type, extension_to_mimetype(path.extension().string()));

			this->result(result);
			this->version(version < 10 ? 11 : version);

			return (*this);
		}

		/**
		 * @brief Returns `true` if this HTTP response's Content-Type is "multipart/form-data";
		 */
		inline bool has_multipart() noexcept
		{
			return has_multipart(*this);
		}

		/**
		 * @brief Get the "multipart/form-data" body content.
		 */
		inline decltype(auto) get_multipart()
		{
			return multipart(*this);
		}

		/**
		 * @brief Get the "multipart/form-data" body content. same as get_multipart
		 */
		inline decltype(auto) multipart()
		{
			return this->get_multipart();
		}

		template<typename Func>
		inline void refresh(Func&& f) { this->refresh_func_ = f; }
		inline void refresh() {
			if (refresh_func_) {
				refresh_func_(*this);
			}
		}

	protected:
		std::filesystem::path                    root_directory_     = std::filesystem::current_path();

		std::function<void()>                    defer_callback_;

		std::function<void(const self& rep)>	 refresh_func_;

		std::shared_ptr<response_defer>    defer_guard_;

		std::weak_ptr<void>                      session_ptr_;
	};
}

namespace bnet::beast::http {
	using web_response = http_response_impl_t<flex_body>;
}

//#include <asio2/base/detail/pop_options.hpp>