/*
 * Copyright (c) 2023 bocai
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

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
			if (!stopped_)
				return false;

			if (!this->works_.empty() || !this->threads_.empty())
				return false;

			for (auto & io : this->ios_) {
				io.context().restart();
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
			if (stopped_)
				return;

			if (this->works_.empty() && this->threads_.empty())
				return;

			if (this->running_in_iopool_threads())
				return;

			stopped_ = true;

			for (auto & io : this->ios_) {
				io.context().stop();
			}

			for (auto & work : this->works_) {
				work.reset();
			}

			for (auto & thread : this->threads_) {
				thread.join();
			}

			this->works_.clear();
			this->threads_.clear();
		}

		inline nio & get(std::size_t index = static_cast<std::size_t>(-1)) {
			return this->ios_[(index == static_cast<std::size_t>(-1) ?
				((++(this->next_)) % this->ios_.size()) : (index % this->ios_.size()))];
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
		//std::mutex  mutex_;
		std::atomic<bool> stopped_ = true;
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
