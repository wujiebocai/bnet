#pragma once

//#include <vector>
//#include <thread>
//#include <mutex>
//#include <chrono>
//#include <type_traits>

namespace bnet::base {
	class nio {
	public:
		nio() : context_(1), strand_(context_) {}
		~nio() = default;

		inline asio::io_context & context() { return this->context_; }
		inline asio::io_context::strand &  strand() { return this->strand_; }

	protected:
		asio::io_context context_;
		asio::io_context::strand strand_;
	};

	class iopool_imp {
	public:
		explicit iopool_imp(std::size_t concurrency = std::thread::hardware_concurrency() * 2)
			: ios_(concurrency == 0 ? std::thread::hardware_concurrency() * 2 : concurrency) {
			this->threads_.reserve(this->ios_.size());
			this->works_.reserve(this->ios_.size());
		}

		~iopool_imp() = default;

		bool start() {
			std::lock_guard<std::mutex> guard(this->mutex_);
			if (!stopped_)
				return false;
			if (!this->works_.empty() || !this->threads_.empty())
				return false;

			for (auto & io : this->ios_) {
				io.context().restart();
			}

			for (auto & io : this->ios_) {
				this->works_.emplace_back(io.context().get_executor());
				// start work thread
				this->threads_.emplace_back([&io]() {
					io.context().run();
				});
			}

			stopped_ = false;

			return true;
		}

		void stop() {
			{
				std::lock_guard<std::mutex> guard(this->mutex_);

				if (stopped_)
					return;

				if (this->works_.empty() && this->threads_.empty())
					return;

				if (this->running_in_iopool_threads())
					return;

				stopped_ = true;
			}

			{
				for (std::size_t i = 0; i < this->ios_.size(); ++i) {
					asio::io_context& ioc = this->ios_.at(i).context();
					ioc.stop();
				}
			}
			{
				std::lock_guard<std::mutex> guard(this->mutex_);

				for (std::size_t i = 0; i < this->works_.size(); ++i) {
					this->works_[i].reset();
				}

				for (auto & thread : this->threads_) {
					thread.join();
				}

				this->works_.clear();
				this->threads_.clear();
			}
		}

		/**
		 * Use to ensure that all nested asio::post(...) events are fully invoked.
		 */
		inline void wait_for_io_context_stopped() {
			// split read and write to avoid deadlock caused by iopool.post([&iopool]() {iopool.stop(); });
			{
				//asio2::shared_locker guard(this->mutex_);

				if (this->running_in_iopool_threads())
					return;
			}

			{
				std::lock_guard<std::mutex> guard(this->mutex_);

				// first reset the acceptor io_context work guard
				if (!this->works_.empty())
					this->works_.front().reset();
			}

			constexpr auto max = std::chrono::milliseconds(10);
			constexpr auto min = std::chrono::milliseconds(1);
/*
			{
				// don't need lock, maybe cause deadlock in client start iopool
				//asio2::shared_locker guard(this->mutex_);

				// second wait indefinitely until the acceptor io_context is stopped
				for (std::size_t i = 0; i < std::size_t(1) && i < this->ios_.size(); ++i) {
					auto t1 = std::chrono::steady_clock::now();
					auto& io = this->ios_[i];
					while (!io.context().stopped()) {
						// the timer may not be canceled successed when using visual
						// studio break point for debugging, so cancel it at each loop

						// must cancel all iots, otherwise maybe cause deaklock like below:
						// the client_ptr->bind_recv has hold the session_ptr, and the session_ptr
						// is in the indexed 1 iot ( not indexed 0 iot ), so if call iot->cancel,
						// the cancel function of indexed 1 iot wont be called, so the stop function
						// of client_ptr won't be called too, so the session_ptr which holded by the
						// client_ptr will can't be destroyed, so the server's acceptor io will 
						// can't be stopped(this means the indexed 0 io can't be stopped).

						//server.bind_accept([](std::shared_ptr<asio2::tcp_session>& session_ptr)
						//{
						//	std::shared_ptr<asio2::tcp_client> client_ptr = std::make_shared<asio2::tcp_client>(
						//		512, 1024, session_ptr->io());
						//
						//	client_ptr->bind_recv([session_ptr](std::string_view data) mutable
						//	{
						//	});
						//
						//	client_ptr->async_start("127.0.0.1", 8888);
						//});

						auto t2 = std::chrono::steady_clock::now();
						auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
						std::this_thread::sleep_for(std::clamp(ms, min, max));
					}
				}
			}
*/
			{
				std::lock_guard<std::mutex> guard(this->mutex_);

				for (std::size_t i = 1; i < this->works_.size(); ++i) {
					this->works_[i].reset();
				}
			}

			{
				// don't need lock, maybe cause deadlock in client start iopool
				//asio2::shared_locker guard(this->mutex_);

				for (std::size_t i = 1; i < this->ios_.size(); ++i) {
					auto t1 = std::chrono::steady_clock::now();
					auto& io = this->ios_[i];
					while (!io.context().stopped()) {
						auto t2 = std::chrono::steady_clock::now();
						auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
						std::this_thread::sleep_for(std::clamp(ms, min, max));
					}
				}
			}
		}

		inline nio & get(std::size_t index = static_cast<std::size_t>(-1)) {
			return this->ios_[index < this->ios_.size() ? index : ((++(this->next_)) % this->ios_.size())];
		}
		
		inline bool running_in_iopool_threads() {
			std::thread::id curr_tid = std::this_thread::get_id();
			for (auto & thread : this->threads_) {
				if (curr_tid == thread.get_id())
					return true;
			}
			return false;
		}

	protected:
		std::vector<std::thread> threads_;
		std::vector<nio> ios_;
		std::vector<asio::executor_work_guard<asio::io_context::executor_type>> works_;
		std::mutex  mutex_;
		bool stopped_ = true;
		std::size_t next_ = 0;
	};

	class iopool {
	public:
		iopool(std::size_t concurrency) : iopool_(concurrency) {}
		~iopool() = default;

	protected:
		iopool_imp iopool_;
	};
}
