/*
 * Copyright (c) 2023 bocai
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include "tool/noncopyable.hpp"

namespace bnet {
	/// default block size
	constexpr unsigned int trunkSize = 64 * 1024;
	/// calculate the number of memory blocks based on size
	/// size
#define trunkCount(size, trunklen) (((size) + trunklen - 1) / trunklen)

	template<typename T>
	concept buffer_is_dynamic = requires(T t) {
		{ t.resize(std::size_t(0)) };
	};

	template <typename btype, unsigned int trunklen, bool isdynamic = buffer_is_dynamic<btype>>
	class bytebuffer;

	template <typename btype, unsigned int trunklen>
	class bytebuffer<btype, trunklen, true> {
	public:
		bytebuffer()
			: max_size_(trunklen)
			, offptr_(0)
			, currptr_(0)
			, buffer_(max_size_)
		{
		}
		//bytebuffer(const bytebuffer&);

		inline void wr_reserve(const unsigned int size) {
			if (wr_size() < size + 8) {
				max_size_ += (trunklen * trunkCount(size + 8, trunklen));
				buffer_.resize(max_size_);
			}
		}

		inline void put(const char *buf, const unsigned int size) {
			wr_reserve(size);
			std::memmove(&buffer_[currptr_], buf, size);
			currptr_ += size;
		}

		inline char *wr_buf() {
			return &buffer_[currptr_];
		}

		inline const char *rd_buf() const {
			return &buffer_[offptr_];
		}

		inline bool rd_ready() const {
			return currptr_ > offptr_;
		}

		inline unsigned int rd_size() const {
			return currptr_ - offptr_;
		}

		inline void rd_flip(unsigned int size) {
			offptr_ += size;
			if (currptr_ > offptr_) {
				unsigned int tmp = currptr_ - offptr_;
				if (offptr_ >= tmp) {
					std::memmove(&buffer_[0], &buffer_[offptr_], tmp);
					offptr_ = 0;
					currptr_ = tmp;
				}
			}
			else {
				offptr_ = 0;
				currptr_ = 0;
			}
		}

		inline unsigned int wr_size() const {
			return max_size_ - currptr_;
		}

		inline void wr_flip(const unsigned int size) {
			currptr_ += size;
		}

		inline void reset() {
			offptr_ = 0;
			currptr_ = 0;
		}

		inline unsigned int maxSize() const {
			return max_size_;
		}

		inline bool is_range(void* pointer) {
			auto beginaddr = &buffer_[0];
			auto endaddr = &buffer_[max_size_];
			if (pointer >= beginaddr && pointer <= endaddr) {
				return true;
			}
			return false;
		}

	private:
		unsigned int max_size_;
		unsigned int offptr_;
		unsigned int currptr_;
		
		btype buffer_;
	};

	template <typename btype, unsigned int trunklen>
	class bytebuffer<btype, trunklen, false> {
	public:
		bytebuffer()
			: max_size_(trunklen)
			, offptr_(0)
			, currptr_(0)
		{
			std::memset(buffer_, 0, sizeof(buffer_));
		}
		//bytebuffer(const bytebuffer&);

		inline void wr_reserve(const unsigned int size) {
			//static cache cannot be extended
			
			assert(wr_size() >= size);
		}

		inline void put(const char *buf, const unsigned int size) {
			wr_reserve(size);
			std::memmove(&buffer_[currptr_], buf, size);
			currptr_ += size;
		}

		inline char *wr_buf() {
			return &buffer_[currptr_];
		}

		inline const char *rd_buf() const {
			return &buffer_[offptr_];
		}

		inline bool rd_ready() const {
			return currptr_ > offptr_;
		}

		inline unsigned int rd_size() const {
			return currptr_ - offptr_;
		}

		inline void rd_flip(unsigned int size) {
			offptr_ += size;
			if (currptr_ > offptr_) {
				unsigned int tmp = currptr_ - offptr_;
				if (offptr_ >= tmp) {
					std::memmove(&buffer_[0], &buffer_[offptr_], tmp);
					offptr_ = 0;
					currptr_ = tmp;
				}
			}
			else {
				offptr_ = 0;
				currptr_ = 0;
			}
		}

		inline unsigned int wr_size() const {
			return max_size_ - currptr_;
		}

		inline void wr_flip(const unsigned int size) {
			currptr_ += size;
		}

		inline void reset() {
			offptr_ = 0;
			currptr_ = 0;
		}

		inline unsigned int maxSize() const {
			return max_size_;
		}

		inline bool is_range(void* pointer) {
			auto beginaddr = &buffer_[0];
			auto endaddr = &buffer_[max_size_];
			if (pointer >= beginaddr && pointer <= endaddr) {
				return true;
			}
			return false;
		}

	private:
		unsigned int max_size_;
		unsigned int offptr_;
		unsigned int currptr_;
		
		btype buffer_;
	};

	template<unsigned int trunklen = trunkSize>
	class dynamic_buffer : public bytebuffer<std::vector<char>, trunklen>
							, private noncopyable {
	public:
		using super = bytebuffer<std::vector<char>, trunklen>;
		dynamic_buffer() : super() {
		}
		~dynamic_buffer() = default;

		dynamic_buffer(const dynamic_buffer& allm) = delete;
		dynamic_buffer& operator=(const dynamic_buffer&) = delete;
	};

	template<unsigned int trunklen = trunkSize>
	class static_buffer : public bytebuffer<char[trunklen], trunklen>
							, private noncopyable {
	public:
		using super = bytebuffer<char[trunklen], trunklen>;
		static_buffer() : super() {
		}
		~static_buffer() = default;

		static_buffer(const static_buffer& allm) = delete;
		static_buffer& operator=(const static_buffer&) = delete;
	};
}

