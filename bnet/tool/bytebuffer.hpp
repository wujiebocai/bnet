#pragma once

//#include <vector>
//#include <string>
//#include <cstring>
//#include <assert.h>

#include "tool/noncopyable.hpp"

namespace bnet {
	/// 默认内存块单位大小
	const unsigned int trunkSize = 64 * 1024;
	/// 根据size计算内存块数量
	/// size 大小
#define trunkCount(size, trunklen) (((size) + trunklen - 1) / trunklen)

	// check缓存是否可以动态扩充
	template<typename T>
	struct buffer_is_dynamic {
	private:
		template<typename U>
		static auto check(bool) -> decltype(std::declval<U>().resize(std::size_t(0)), std::true_type());
		template<typename U>
		static std::false_type check(...);
	public:
		static constexpr bool value = std::is_same_v<decltype(check<T>(true)), std::true_type>;
	};

	template <typename _type, unsigned int trunklen, bool isdynamic = buffer_is_dynamic<_type>::value>
	class bytebuffer;

	template <typename _type, unsigned int trunklen>
	class bytebuffer<_type, trunklen, true> {
	public:
		bytebuffer()
			: _maxSize(trunklen)
			, _offPtr(0)
			, _currPtr(0)
			, _buffer(_maxSize)
		{
		}
		//bytebuffer(const bytebuffer&);

		inline void wr_reserve(const unsigned int size) {
			if (wr_size() < size + 8) {
				_maxSize += (trunklen * trunkCount(size + 8, trunklen));
				_buffer.resize(_maxSize);
			}
		}

		inline void put(const char *buf, const unsigned int size) {
			wr_reserve(size);
			std::memmove(&_buffer[_currPtr], buf, size);
			_currPtr += size;
		}

		inline char *wr_buf() {
			return &_buffer[_currPtr];
		}

		inline const char *rd_buf() const {
			return &_buffer[_offPtr];
		}

		inline bool rd_ready() const {
			return _currPtr > _offPtr;
		}

		inline unsigned int rd_size() const {
			return _currPtr - _offPtr;
		}

		inline void rd_flip(unsigned int size) {
			_offPtr += size;
			if (_currPtr > _offPtr) {
				unsigned int tmp = _currPtr - _offPtr;
				if (_offPtr >= tmp) {
					std::memmove(&_buffer[0], &_buffer[_offPtr], tmp);
					_offPtr = 0;
					_currPtr = tmp;
				}
			}
			else {
				_offPtr = 0;
				_currPtr = 0;
			}
		}

		inline unsigned int wr_size() const {
			return _maxSize - _currPtr;
		}

		inline void wr_flip(const unsigned int size) {
			_currPtr += size;
		}

		inline void reset() {
			_offPtr = 0;
			_currPtr = 0;
		}

		inline unsigned int maxSize() const {
			return _maxSize;
		}

		inline bool is_range(void* pointer) {
			auto beginaddr = &_buffer[0];
			auto endaddr = &_buffer[_maxSize];
			if (pointer >= beginaddr && pointer <= endaddr) {
				return true;
			}
			return false;
		}

	private:
		unsigned int _maxSize;
		unsigned int _offPtr;
		unsigned int _currPtr;
		/// 缓冲区
		_type _buffer;
	};

	template <typename _type, unsigned int trunklen>
	class bytebuffer<_type, trunklen, false> {
	public:
		bytebuffer()
			: _maxSize(trunklen)
			, _offPtr(0)
			, _currPtr(0)
		{
			std::memset(_buffer, 0, sizeof(_buffer));
		}
		//bytebuffer(const bytebuffer&);

		inline void wr_reserve(const unsigned int size) {
			//静态缓存不能扩充
			//如果需求的静态缓冲大小小于需求的缓冲大小，肯定会导致栈溢出
			//这里直接退出运行是最安全的做法
			assert(wr_size() >= size);
		}

		inline void put(const char *buf, const unsigned int size) {
			wr_reserve(size);
			std::memmove(&_buffer[_currPtr], buf, size);
			_currPtr += size;
		}

		inline char *wr_buf() {
			return &_buffer[_currPtr];
		}

		inline const char *rd_buf() const {
			return &_buffer[_offPtr];
		}

		inline bool rd_ready() const {
			return _currPtr > _offPtr;
		}

		inline unsigned int rd_size() const {
			return _currPtr - _offPtr;
		}

		inline void rd_flip(unsigned int size) {
			_offPtr += size;
			if (_currPtr > _offPtr) {
				unsigned int tmp = _currPtr - _offPtr;
				if (_offPtr >= tmp) {
					std::memmove(&_buffer[0], &_buffer[_offPtr], tmp);
					_offPtr = 0;
					_currPtr = tmp;
				}
			}
			else {
				_offPtr = 0;
				_currPtr = 0;
			}
		}

		inline unsigned int wr_size() const {
			return _maxSize - _currPtr;
		}

		inline void wr_flip(const unsigned int size) {
			_currPtr += size;
		}

		inline void reset() {
			_offPtr = 0;
			_currPtr = 0;
		}

		inline unsigned int maxSize() const {
			return _maxSize;
		}

		inline bool is_range(void* pointer) {
			auto beginaddr = &_buffer[0];
			auto endaddr = &_buffer[_maxSize];
			if (pointer >= beginaddr && pointer <= endaddr) {
				return true;
			}
			return false;
		}

	private:
		unsigned int _maxSize;
		unsigned int _offPtr;
		unsigned int _currPtr;
		/// 缓冲区
		_type _buffer;
	};

	// 动态内存的缓冲区，可以动态扩展缓冲区大小
	template<unsigned int trunklen = trunkSize>
	class t_buffer_cmdqueue : public bytebuffer<std::vector<char >, trunklen>
							, private noncopyable {
	public:
		using super = bytebuffer<std::vector<char >, trunklen>;
		t_buffer_cmdqueue() : super() {
		}
		~t_buffer_cmdqueue() = default;

		t_buffer_cmdqueue(const t_buffer_cmdqueue& allm) = delete;
		t_buffer_cmdqueue& operator=(const t_buffer_cmdqueue&) = delete;
	};

	// 	以栈空间数组的方式来分配内存,用于一些临时变量的获取
	template<unsigned int trunklen = trunkSize>
	class t_static_cmdqueue : public bytebuffer<char[trunklen], trunklen>
							, private noncopyable {
	public:
		using super = bytebuffer<char[trunklen], trunklen>;
		t_static_cmdqueue() : super() {
		}
		~t_static_cmdqueue() = default;

		t_static_cmdqueue(const t_static_cmdqueue& allm) = delete;
		t_static_cmdqueue& operator=(const t_static_cmdqueue&) = delete;
	};
}

