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

	inline nio **get_current() {
 		static thread_local nio *current = nullptr;
  		return &current;
	}

	class iopool_imp {
	public:
		explicit iopool_imp(std::size_t concurrency = std::thread::hardware_concurrency() * 2)
			: ios_(concurrency == 0 ? std::thread::hardware_concurrency() * 2 : concurrency) {
			this->threads_.reserve(this->ios_.size());
			this->works_.reserve(this->ios_.size());
		}

		//~iopool_imp() = default;

		bool start() {
			bool estop = false;
    		bool ok = stop_.compare_exchange_strong(estop, true);
    		if (!ok) {
      			return false;
    		}

			for (auto & io : this->ios_) {
				io.context().restart();
				this->works_.emplace_back(io.context().get_executor());
				// start work thread
				this->threads_.emplace_back([&io]() {
					[[maybe_unused]] auto ctx = get_current();
            		*ctx = &io;
					io.context().run();
				});
			}

			return true;
		}

		void stop(bool force = false) {
			if (*(get_current()) != nullptr)
				return;
			
			bool estop = true;
    		bool ok = stop_.compare_exchange_strong(estop, false);

			if (force) {
				for (auto & io : this->ios_) {
					io.context().stop();
				}
			}

			for (auto & work : this->works_) {
				work.reset();
			}
			this->works_.clear();

    		if (!ok) {
				// clear all unfinished work
        		for (auto &io : ios_) {
          			io.context().run();
				}
      			return;
    		}

			for (auto & thread : this->threads_) {
				thread.join();
			}

			this->threads_.clear();
		}

		~iopool_imp() {
    		if (!has_stop()) stop(true);
  		}

  		std::size_t pool_size() const noexcept { return ios_.size(); }

 	 	bool has_stop() const { return works_.empty(); }

		inline nio & get(std::size_t index = static_cast<std::size_t>(-1)) {
			return this->ios_[(index == static_cast<std::size_t>(-1) ?
				((next_.fetch_add(1, std::memory_order::relaxed)) % this->ios_.size()) : (index % this->ios_.size()))];
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
		std::atomic<bool> stop_ = false;
		std::atomic<std::size_t> next_ = 0;
	};

	class iopool {
	public:
		iopool(std::size_t concurrency) : iopool_(concurrency) {}
		~iopool() = default;

	protected:
		iopool_imp iopool_;
	};
}
