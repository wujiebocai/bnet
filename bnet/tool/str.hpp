/*
 * Copyright (c) 2023 bocai
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

namespace bnet::tool {
	template<base::is_string_view_c T>
	inline auto conv_to_basic_string(T&& v) {
		using CharT = typename base::unqualified_t<T>::value_type;
		return std::basic_string<CharT>{ v.data(), v.size() };
	}

	template<base::is_string_c T>
	inline auto conv_to_basic_string(T&& v) {
		return std::forward<T>(v);
	}

	template<base::is_char_c T>
	inline auto conv_to_basic_string(T&& v) {
		return std::basic_string<base::unqualified_t<T>>{ 1, v };
	}

	template<base::is_arithmetic_c T>
	inline auto conv_to_basic_string(T&& v) {
		return std::to_string(v);
	}

	template<base::is_c_str_c T>
	inline auto conv_to_basic_string(T&& v) {
		using CharT = base::unqualified_t<std::remove_pointer_t<std::remove_all_extents_t<base::unqualified_t<T>>>>;
		if constexpr (std::is_pointer_v<T>) 
			return (v ? std::basic_string<CharT>{ v } : std::basic_string<CharT>{});
		else 
			return std::basic_string<CharT>{ v };
	}

	template<base::is_string_view_c T>
	inline auto conv_to_basic_string_view(T&& v) {
		//return std::forward<T>(v);
		return v;
	}

	template<base::is_string_c T>
	inline auto conv_to_basic_string_view(T&& v) {
		using CharT = typename base::unqualified_t<T>::value_type;
		return std::basic_string_view<CharT>{ v };
	}

	template<base::is_char_c T>
	inline auto conv_to_basic_string_view(T&& v) {
		return std::basic_string_view<base::unqualified_t<T>>{ std::addressof(v), 1 };
	}
	
	template<base::is_c_str_c T>
	inline auto conv_to_basic_string_view(T&& v) {
		using CharT = base::unqualified_t<std::remove_pointer_t<std::remove_all_extents_t<base::unqualified_t<T>>>>;
		if constexpr (std::is_pointer_v<T>) 
			return (v ? std::basic_string_view<CharT>{ v } : std::basic_string_view<CharT>{});
		else 
			return std::basic_string_view<CharT>{ v };
	}

	/**
	 * @brief Converts any datatype into std::string.
	 * @tparam T
	 * @param v - will be converted into std::string.
	 * @return Converted value as std::string.
	 */
	template<typename T>
	inline std::string to_string(T&& v) {
		return conv_to_basic_string(v);
	}

	/**
	 * @brief Converts any datatype into std::string_view.
	 * @tparam T
	 * @param v - will be converted into std::string_view.
	 * @return Converted value as std::string_view.
	 */
	template<typename T>
	inline std::string_view to_string_view(const T& v) {
		return conv_to_basic_string_view(v);
	}

	/**
	 * @brief Converts iterator range into std::string_view.
	 * @tparam T
	 * @param v - will be converted into std::string_view.
	 * @return Converted value as std::string_view.
	 */
	template<typename Iterator>
	inline std::string_view to_string_view(const Iterator& first, const Iterator& last) {
		using iter_type = typename base::unqualified_t<Iterator>;
		using diff_type = typename std::iterator_traits<iter_type>::difference_type;

		diff_type n = std::distance(first, last);

		if (n <= static_cast<diff_type>(0)) {
			return std::string_view{};
		}

		if constexpr (std::is_pointer_v<iter_type>) {
			return { first, static_cast<std::string_view::size_type>(n) };
		}
		else {
			return { first.operator->(), static_cast<std::string_view::size_type>(n) };
		}
	}

	/**
	 * @brief Converts any datatype into a numeric.
	 * @tparam IntegerType - integer or floating
	 * @param v - will be converted into numeric.
	 * @return Converted value as numeric.
	 */
	template<typename IntegerType, typename T>
	inline IntegerType to_numeric(T&& v) noexcept {
		using type = typename base::unqualified_t<T>;

		if /**/ constexpr (std::is_integral_v<type>) {
			return static_cast<IntegerType>(v);
		}
		else if constexpr (std::is_floating_point_v<type>) {
			return static_cast<IntegerType>(v);
		}
		else {
			std::string s = to_string(std::forward<T>(v));
			int rx = 10;
			if (s.size() >= std::size_t(2) && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
				rx = 16;
			return static_cast<IntegerType>(std::strtoull(s.data(), nullptr, rx));
		}
	}
///////////////////////////////////////////////////////////////////////////////////////////////
    template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator>& to_lower(std::basic_string<CharT, Traits, Allocator>& str) {
		std::transform(str.begin(), str.end(), str.begin(), [](CharT c) -> CharT {
			return static_cast<CharT>(std::tolower(c));
		});

		return str;
	}

    template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator>& to_upper(std::basic_string<CharT, Traits, Allocator>& str) {
		std::transform(str.begin(), str.end(), str.begin(), [](CharT c) -> CharT {
			return static_cast<CharT>(std::toupper(c));
		});

		return str;
	}

    /**
	 * @brief Converts the first character of a string to uppercase letter and lowercases all other characters, if any.
	 * @param str - input string to be capitalized.
	 * @return A string with the first letter capitalized and all other characters lowercased.
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator>& capitalize(std::basic_string<CharT, Traits, Allocator>& str) {
		to_lower(str);

		if (!str.empty()) {
			str.front() = static_cast<CharT>(std::toupper(str.front()));
		}

		return str;
	}

    /**
	 * @brief Converts only the first character of a string to uppercase letter, all other characters stay unchanged.
	 * @param str - input string to be modified.
	 * @return A string with the first letter capitalized. All other characters stay unchanged.
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator>& capitalize_first_char(
		std::basic_string<CharT, Traits, Allocator>& str) {
		if (!str.empty()) {
			str.front() = static_cast<CharT>(std::toupper(str.front()));
		}

		return str;
	}

    /**
	 * @brief Checks if input string str contains specified substring.
	 * @param str - string to be checked.
	 * @param substring - searched substring or character.
	 * @return True if substring or character was found in str, false otherwise.
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	inline bool contains(std::basic_string_view<CharT, Traits> str, std::basic_string_view<CharT, Traits> substring) {
		return str.find(substring) != std::string_view::npos;
	}
    
    /**
	 * @brief trim each space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator>& trim_all(std::basic_string<CharT, Traits, Allocator>& str) {
		// https://zh.cppreference.com/w/cpp/algorithm/remove
		str.erase(std::remove_if(str.begin(), str.end(), [](int x) {return std::isspace(x); }), str.end());
		return str;
	}

    /**
	 * @brief trim left space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator>& trim_left(std::basic_string<CharT, Traits, Allocator>& str) {
		str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch) { return !std::isspace(ch); }));
		return str;
	}
	/**
	 * @brief trim left space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	inline std::basic_string_view<CharT, Traits>& trim_left(std::basic_string_view<CharT, Traits>& str) {
		if (str.empty())
			return str;
		using size_type = typename std::basic_string_view<CharT, Traits>::size_type;
		size_type pos = 0;
		for (; pos < str.size(); ++pos) {
			if (!std::isspace(static_cast<unsigned char>(str[pos])))
				break;
		}
		str.remove_prefix(pos);
		return str;
	}

    /**
	 * @brief trim right space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator>& trim_right(std::basic_string<CharT, Traits, Allocator>& str) {
		str.erase(std::find_if(str.rbegin(), str.rend(), [](int ch) { return !std::isspace(ch); }).base(), str.end());
		return str;
	}
	/**
	 * @brief trim right space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	inline std::basic_string_view<CharT, Traits>& trim_right(std::basic_string_view<CharT, Traits>& str) {
		if (str.empty())
			return str;
		using size_type = typename std::basic_string_view<CharT, Traits>::size_type;
		size_type pos = str.size() - 1;
		for (; pos != size_type(-1); pos--) {
			if (!std::isspace(static_cast<unsigned char>(str[pos])))
				break;
		}
		str.remove_suffix(str.size() - pos - 1);
		return str;
	}

    /**
	 * @brief trim left and right space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator>& trim_both(std::basic_string<CharT, Traits, Allocator>& str) {
		trim_left(str);
		trim_right(str);
		return str;
	}
	/**
	 * @brief trim left and right space character of the string: space \t \r \n and so on
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	inline std::basic_string_view<CharT, Traits>& trim_both(std::basic_string_view<CharT, Traits>& str) {
		trim_left(str);
		trim_right(str);
		return str;
	}

    /**
	 * @brief Replaces (in-place) the first occurrence of target with replacement.
	 * @param str - input string that will be modified.
	 * @param target - substring that will be replaced with replacement.
	 * @param replacement - substring that will replace target.
	 * @return Replacemented input string.
	 */
	template<
		class String1,
		class String2,
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator>& replace_first(
		std::basic_string<CharT, Traits, Allocator>& str,
		const String1& target,
		const String2& replacement) {
		auto t = to_basic_string_view(target);
		auto r = to_basic_string_view(replacement);

		const std::size_t start_pos = str.find(t);
		if (start_pos == std::string::npos) {
			return str;
		}

		str.replace(start_pos, t.length(), r);
		return str;
	}

    /**
	 * @brief Replaces (in-place) all occurrences of target with replacement.
	 * @param str - input string that will be modified.
	 * @param target - substring that will be replaced with replacement.
	 * @param replacement - substring that will replace target.
	 * @return Replacemented input string.
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline std::basic_string<CharT, Traits, Allocator>& replace_all(
		std::basic_string<CharT, Traits, Allocator>& str,
		const std::basic_string<CharT, Traits, Allocator>& target,
		const std::basic_string<CharT, Traits, Allocator>& replacement) {
		if (target.empty()) {
			return str;
		}

		std::size_t start_pos = 0;

		while ((start_pos = str.find(target, start_pos)) != std::string::npos) {
			str.replace(start_pos, target.length(), replacement);
			start_pos += replacement.length();
		}

		return str;
	}

    template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
    bool has_prefix(std::basic_string<CharT, Traits, Allocator>& s, std::basic_string<CharT, Traits, Allocator>& prefix) {
        return (s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix);
    }
    template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
    bool has_suffix(std::basic_string<CharT, Traits, Allocator>& s, std::basic_string<CharT, Traits, Allocator>& suffix) {
        return (s.size() >= suffix.size() && s.substr(s.size() - suffix.size()) == suffix);
    }

	/**
	 * @brief Split string
	 * @param s - input string that will be splited.
	 * @param delimiters - delimiters.
	 * @param count - == 0 no split, > 0 split to count, < 0 split the whole string.
	 * @return Converted value as std::string_view.
	 */
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

    /**
	 * @brief Joins all elements of std::vector tokens of arbitrary datatypes
	 *        into one string with delimiter delim.
	 * @tparam T - arbitrary datatype.
	 * @param tokens - vector of tokens.
	 * @param delim - the delimiter.
	 * @return string with joined elements of vector tokens with delimiter delim.
	 */
	template<
		class T,
		class CharT
	>
	inline auto join(const std::vector<T>& tokens, const std::basic_string<CharT>& delim) {
		std::basic_ostringstream<CharT> result;
		for (auto it = tokens.begin(); it != tokens.end(); ++it) {
			if (it != tokens.begin()) {
				result << delim;
			}

			result << *it;
		}

		return result.str();
	}

    /**
	 * @brief Inplace removal of all empty strings in a vector<string>
	 * @param tokens - vector of strings.
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline void drop_empty(std::vector<std::basic_string<CharT, Traits, Allocator>>& tokens) {
		auto last = std::remove_if(tokens.begin(), tokens.end(),
			[](const std::basic_string<CharT, Traits, Allocator>& s) { return s.empty(); });
		tokens.erase(last, tokens.end());
	}

    /**
	 * @brief Inplace removal of all duplicate strings in a vector<string> where order is not to be maintained
	 *        Taken from: C++ Primer V5
	 * @param tokens - vector of strings.
	 * @return vector of non-duplicate tokens.
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	inline void drop_duplicate(std::vector<std::basic_string<CharT, Traits, Allocator>>& tokens) {
		std::sort(tokens.begin(), tokens.end());
		auto end_unique = std::unique(tokens.begin(), tokens.end());
		tokens.erase(end_unique, tokens.end());
	}

    /**
	 * @brief Sort input std::vector<string> strs in ascending order.
	 * @param strs - std::vector<string> to be checked.
	 */
	template<typename T>
	inline void sorting_ascending(std::vector<T>& strs) {
		std::sort(strs.begin(), strs.end());
	}

	/**
	 * @brief Sorted input std::vector<string> strs in descending order.
	 * @param strs - std::vector<string> to be checked.
	 */
	template<typename T>
	inline void sorting_descending(std::vector<T>& strs) {
		std::sort(strs.begin(), strs.end(), std::greater<T>());
	}

    /**
	 * @brief Reverse input std::vector<string> strs.
	 * @param strs - std::vector<string> to be checked.
	 */
	template<typename T>
	inline void reverse_inplace(std::vector<T>& strs) {
		std::reverse(strs.begin(), strs.end());
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

	/**
	 * @brief Returns `true` if two strings are equal, using a case-insensitive comparison.
	 */
	template<
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	inline bool str_equals(
		std::basic_string_view<CharT, Traits> str1,
		std::basic_string_view<CharT, Traits> str2) noexcept {
		auto n = str1.size();
		if (str2.size() != n)
			return false;
		auto p1 = str1.data();
		auto p2 = str2.data();
		CharT a, b;
		// fast loop
		while (n--) {
			a = *p1++;
			b = *p2++;
			if (a != b)
				goto slow;
		}
		return true;
	slow:
		do {
			if (std::tolower(a) != std::tolower(b))
				return false;
			a = *p1++;
			b = *p2++;
		} while (n--);
		return true;
	}

	/**
	 * @brief Returns `true` if two strings are equal, using a case-insensitive comparison.
	*/
	template<class String1, class String2>
	inline bool str_equals(const String1& str1, const String2& str2) noexcept {
		return str_equals(conv_to_basic_string_view(str1), conv_to_basic_string_view(str2));
	}

	bool has_prefix(std::string& s, std::string& prefix) {
        return (s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix);
    }
    bool has_suffix(std::string& s, std::string& suffix) {
        return (s.size() >= suffix.size() && s.substr(s.size() - suffix.size()) == suffix);
    }
}