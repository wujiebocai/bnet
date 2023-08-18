#pragma once

namespace bnet::tool {
	template <typename Enum>
	inline constexpr auto enum_to_int(Enum const value) -> typename std::underlying_type<Enum>::type {
		return static_cast<typename std::underlying_type<Enum>::type>(value);
	}
/*
	// 修剪字符串的每个空格字符：空格 \t \r \n 等
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	std::basic_string<CharT, Traits, Allocator>& trim_all(
		std::basic_string<CharT, Traits, Allocator>& s) {
		if (s.empty())
			return s;
		using size_type = typename std::basic_string<CharT, Traits, Allocator>::size_type;
		for (size_type i = s.size() - 1; i != size_type(-1); i--) {
			if (std::isspace(static_cast<unsigned char>(s[i]))) {
				s.erase(i, 1);
			}
		}
		return s;
	}
	// 修剪字符串的左空格字符：空格 \t \r \n 等 
	template<class CharT, class Traits = std::char_traits<CharT>>
	std::basic_string_view<CharT, Traits>& trim_left(std::basic_string_view<CharT, Traits>& s) {
		if (s.empty())
			return s;
		using size_type = typename std::basic_string_view<CharT, Traits>::size_type;
		size_type pos = 0;
		for (; pos < s.size(); ++pos) {
			if (!std::isspace(static_cast<unsigned char>(s[pos])))
				break;
		}
		s.remove_prefix(pos);
		return s;
	}
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	std::basic_string<CharT, Traits, Allocator>& trim_left(
		std::basic_string<CharT, Traits, Allocator>& s) {
		if (s.empty())
			return s;
		using size_type = typename std::basic_string<CharT, Traits, Allocator>::size_type;
		size_type pos = 0;
		for (; pos < s.size(); ++pos) {
			if (!std::isspace(static_cast<unsigned char>(s[pos])))
				break;
		}
		s.erase(0, pos);
		return s;
	}
	// 修剪字符串的右空格字符：空格 \t \r \n 等 
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	std::basic_string<CharT, Traits, Allocator>& trim_right(
		std::basic_string<CharT, Traits, Allocator>& s) {
		if (s.empty())
			return s;
		using size_type = typename std::basic_string<CharT, Traits, Allocator>::size_type;
		size_type pos = s.size() - 1;
		for (; pos != size_type(-1); pos--) {
			if (!std::isspace(static_cast<unsigned char>(s[pos])))
				break;
		}
		s.erase(pos + 1);
		return s;
	}
	template<class CharT, class Traits = std::char_traits<CharT>>
	std::basic_string_view<CharT, Traits>& trim_right(std::basic_string_view<CharT, Traits>& s) {
		if (s.empty())
			return s;
		using size_type = typename std::basic_string_view<CharT, Traits>::size_type;
		size_type pos = s.size() - 1;
		for (; pos != size_type(-1); pos--) {
			if (!std::isspace(static_cast<unsigned char>(s[pos])))
				break;
		}
		s.remove_suffix(s.size() - pos - 1);
		return s;
	}
	// 修剪字符串的左右空格字符：空格\t\r\n等
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	std::basic_string<CharT, Traits, Allocator>& trim_both(
		std::basic_string<CharT, Traits, Allocator>& s) {
		trim_left(s);
		trim_right(s);
		return s;
	}
	template<class CharT, class Traits = std::char_traits<CharT>>
	std::basic_string_view<CharT, Traits>& trim_both(std::basic_string_view<CharT, Traits>& s) {
		trim_left(s);
		trim_right(s);
		return s;
	}
	// 分割
	// count == 0 返回nil
	// count > 0 返回count个字串
	// count < 0 全部分割字符串
 	template<class String, class Delimiter>
	inline std::vector<String> split(const String& s, const Delimiter& delimiters = " ", int count = -1) {
		using size_type = typename String::size_type;
		std::vector<String> tokens;
		if (count == 0) {
			return tokens;
		}
		
		size_type last_pos = s.find_first_not_of(delimiters, 0);
		size_type pos = s.find_first_of(delimiters, last_pos);
		while (String::npos != pos || String::npos != last_pos) {
			if (count > 0) {
				count--;
				if (count == 0) {
					tokens.emplace_back(s.substr(last_pos));
					break;
				}
			}

			tokens.emplace_back(s.substr(last_pos, pos - last_pos));
			last_pos = s.find_first_not_of(delimiters, pos);
			pos = s.find_first_of(delimiters, last_pos);
		}
		return tokens;
	}

	template<class T, bool Flag = std::is_trivial_v<T>>
	struct char_type;
	template<class T>
	struct char_type<T, true> {
		using type = T;
	};
	template<class T>
	struct char_type<T, false> {
		using type = typename T::value_type;
	};
	template<typename = void>
	inline char ascii_tolower(char c) {
		return char(((static_cast<unsigned>(c) - 65U) < 26) ? c + 'a' - 'A' : c);
	}

	template<class String1, class String2>
	inline std::size_t ifind(const String1& src, const String2& dest, typename String1::size_type pos = 0) {
		using str2_type = std::remove_reference_t<std::remove_cv_t<String2>>;
		using raw2_type = std::remove_pointer_t<std::remove_all_extents_t<str2_type>>;

		std::basic_string_view<typename char_type<raw2_type>::type> des{ dest };

		if (pos >= src.size() || des.empty())
			return String1::npos;

		// Outer loop
		for (auto OuterIt = std::next(src.begin(), pos); OuterIt != src.end(); ++OuterIt) {
			auto InnerIt = OuterIt;
			auto SubstrIt = des.begin();
			for (; InnerIt != src.end() && SubstrIt != des.end(); ++InnerIt, ++SubstrIt) {
				if (std::tolower(*InnerIt) != std::tolower(*SubstrIt))
					break;
			}

			// Substring matching succeeded
			if (SubstrIt == des.end())
				return std::distance(src.begin(), OuterIt);
		}

		return String1::npos;
	}
*/
	// Returns true if the current machine is little endian
	template<typename = void>
	inline bool is_little_endian() noexcept {
		static std::int32_t test = 1;
		return (*reinterpret_cast<std::int8_t*>(std::addressof(test)) == 1);
	}

	/**
	 * Swaps the order of bytes for some chunk of memory
	 * @param data - The data as a uint8_t pointer
	 * @tparam DataSize - The true size of the data
	 */
	template <std::size_t DataSize>
	inline void swap_bytes(std::uint8_t * data) noexcept {
		for (std::size_t i = 0, end = DataSize / 2; i < end; ++i)
			std::swap(data[i], data[DataSize - i - 1]);
	}

	template<class T, class Pointer>
	inline void write(Pointer& p, T v) noexcept {
		if constexpr (int(sizeof(T)) > 1) {
			// MSDN: The htons function converts a u_short from host to TCP/IP network byte order (which is big-endian).
			// ** This mean the network byte order is big-endian **
			if (is_little_endian()) {
				swap_bytes<sizeof(T)>(reinterpret_cast<std::uint8_t *>(std::addressof(v)));
			}

			std::memcpy((void*)p, (const void*)&v, sizeof(T));
		}
		else {
			static_assert(sizeof(T) == std::size_t(1));

			*p = std::decay_t<std::remove_pointer_t<std::remove_cv_t<std::remove_reference_t<Pointer>>>>(v);
		}

		p += sizeof(T);
	}

	template<class T, class Pointer>
	inline T read(Pointer& p) noexcept {
		T v{};

		if constexpr (int(sizeof(T)) > 1) {
			std::memcpy((void*)&v, (const void*)p, sizeof(T));

			// MSDN: The htons function converts a u_short from host to TCP/IP network byte order (which is big-endian).
			// ** This mean the network byte order is big-endian **
			if (is_little_endian()) {
				swap_bytes<sizeof(T)>(reinterpret_cast<std::uint8_t *>(std::addressof(v)));
			}
		}
		else {
			static_assert(sizeof(T) == std::size_t(1));

			v = T(*p);
		}

		p += sizeof(T);

		return v;
	}
}

namespace std
{
	/**
	 * BKDR Hash Function
	 */
	inline std::size_t bkdr_hash(const unsigned char* const p, std::size_t size)
	{
		std::size_t v = 0;
		for (std::size_t i = 0; i < size; ++i)
		{
			v = v * 131 + static_cast<std::size_t>(p[i]);
		}
		return v;
	}
	/**
	 * Fnv1a Hash Function
	 * Reference from Visual c++ implementation, see vc++ std::hash
	 */
	template<typename T>
	inline T fnv1a_hash(const unsigned char* const p, const T size) noexcept
	{
		static_assert(sizeof(T) == 4 || sizeof(T) == 8, "Must be 32 or 64 digits");
		T v;
		if constexpr (sizeof(T) == 4)
			v = 2166136261u;
		else
			v = 14695981039346656037ull;
		for (T i = 0; i < size; ++i)
		{
			v ^= static_cast<T>(p[i]);
			if constexpr (sizeof(T) == 4)
				v *= 16777619u;
			else
				v *= 1099511628211ull;
		}
		return (v);
	}
	/*
	template<> struct hash<asio::ip::udp::endpoint>
	{
		typedef asio::ip::udp::endpoint argument_type;
		typedef std::size_t result_type;
		inline result_type operator()(argument_type const& s) const noexcept
		{
			//return std::hash<std::string_view>()(std::string_view{
			//	reinterpret_cast<std::string_view::const_pointer>(&s),sizeof(argument_type) });
			return bkdr_hash((const unsigned char*)(&s), sizeof(argument_type));
		}
	};

	template<> struct hash<asio::ip::tcp::endpoint>
	{
		typedef asio::ip::tcp::endpoint argument_type;
		typedef std::size_t result_type;
		inline result_type operator()(argument_type const& s) const noexcept
		{
			//return std::hash<std::string_view>()(std::string_view{
			//	reinterpret_cast<std::string_view::const_pointer>(&s),sizeof(argument_type) });
			return bkdr_hash((const unsigned char*)(&s), sizeof(argument_type));
		}
	};
	*/
}

