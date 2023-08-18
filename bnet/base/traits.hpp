#pragma once

#include <type_traits>

namespace bnet::base {
    template <typename T>
	using unqualified = std::remove_cv<std::remove_reference_t<T>>;
	template <typename T>
	using unqualified_t = typename unqualified<T>::type;

	template<class T>
	struct is_shared_ptr : std::false_type {};
	template<class T>
	struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};
	template<class T>
	constexpr bool is_shared_ptr_v = is_shared_ptr<T>::value;

	template <bool B>
	using boolean = std::integral_constant<bool, B>;
	template <bool B>
	constexpr inline bool boolean_v = boolean<B>::value;

	template <typename T>
	using neg = boolean<!T::value>;
	template <typename T>
	constexpr inline bool neg_v = neg<T>::value;

	template <typename T, typename...>
	struct all_same : std::true_type { };
	template <typename T, typename U, typename... Args>
	struct all_same<T, U, Args...> : std::integral_constant<bool, std::is_same<T, U>::value && all_same<T, Args...>::value> { };

	template <typename T, typename...>
	struct any_same : std::false_type { };
	template <typename T, typename U, typename... Args>
	struct any_same<T, U, Args...> : std::integral_constant<bool, std::is_same<T, U>::value || any_same<T, Args...>::value> { };
	template <typename T, typename... Args>
	constexpr inline bool any_same_v = any_same<T, Args...>::value;

	template <typename T, typename CharT = char>
	using is_string_literal_array_of = boolean<std::is_array_v<T> && std::is_same_v<std::remove_all_extents_t<T>, CharT>>;
	template <typename T, typename CharT = char>
	constexpr inline bool is_string_literal_array_of_v = is_string_literal_array_of<T, CharT>::value;

	template <typename T>
	using is_string_literal_array = boolean<std::is_array_v<T> && any_same_v<std::remove_all_extents_t<T>, 
		char,
	#if defined(__cpp_lib_char8_t)
		char8_t, 
	#endif
		char16_t, char32_t, wchar_t>>;
	template <typename T>
	constexpr inline bool is_string_literal_array_v = is_string_literal_array<T>::value;

	template <typename T, typename CharT>
	struct is_string_of : std::false_type { };
	template <typename CharT, typename CharTargetT, typename TraitsT, typename AllocT>
	struct is_string_of<std::basic_string<CharT, TraitsT, AllocT>, CharTargetT> : std::is_same<CharT, CharTargetT> { };
	template <typename T, typename CharT>
	constexpr inline bool is_string_of_v = is_string_of<T, CharT>::value;

	template <typename T, typename CharT>
	struct is_string_view_of : std::false_type { };
	template <typename CharT, typename CharTargetT, typename TraitsT>
	struct is_string_view_of<std::basic_string_view<CharT, TraitsT>, CharTargetT> : std::is_same<CharT, CharTargetT> { };
	template <typename T, typename CharT>
	constexpr inline bool is_string_view_of_v = is_string_view_of<T, CharT>::value;

	namespace base_detail {
		template <typename T, template <typename...> class Templ>
		struct is_specialization_of : std::false_type { };
		template <typename... T, template <typename...> class Templ>
		struct is_specialization_of<Templ<T...>, Templ> : std::true_type { };
	}
	template <typename T, template <typename...> class Templ>
	using is_specialization_of = base_detail::is_specialization_of<std::remove_cv_t<T>, Templ>;
	template <typename T, template <typename...> class Templ>
	inline constexpr bool is_specialization_of_v = is_specialization_of<std::remove_cv_t<T>, Templ>::value;

	template <typename T>
	using is_string_like
		= boolean<is_specialization_of_v<T, std::basic_string> || is_specialization_of_v<T, std::basic_string_view> || is_string_literal_array_v<T>>;
	template <typename T>
	constexpr inline bool is_string_like_v = is_string_like<T>::value;

	template <typename T, typename CharT = char>
	using is_string_constructible = boolean<
		is_string_literal_array_of_v<T,
		     CharT> || std::is_same_v<T, const CharT*> || std::is_same_v<T, CharT> || is_string_of_v<T, CharT> || std::is_same_v<T, std::initializer_list<CharT>> || is_string_view_of_v<T, CharT> || std::is_null_pointer_v<T>>;
	template <typename T, typename CharT = char>
	constexpr inline bool is_string_constructible_v = is_string_constructible<T, CharT>::value;

	template <typename T>
	using is_string_like_or_constructible = boolean<is_string_like_v<T> || is_string_constructible_v<T>>;

	template <typename T, typename Char>
	using is_c_str_of = std::disjunction<std::is_same<T, const Char*>, std::is_same<T, Char const* const>, std::is_same<T, Char*>, is_string_literal_array_of<T, Char>>;
	template <typename T, typename Char>
	constexpr inline bool is_c_str_of_v = is_c_str_of<T, Char>::value;

	//template <typename T>
	//using is_c_str = is_c_str_of<T, char>;
	//template <typename T>
	//constexpr inline bool is_c_str_v = is_c_str<T>::value;

	template <typename T, typename Char>
	using is_c_str_or_string_of = std::disjunction<is_c_str_of<T, Char>, is_string_of<T, Char>>;
	template <typename T, typename Char>
	constexpr inline bool is_c_str_or_string_of_v = is_c_str_or_string_of<T, Char>::value;

	template <typename T>
	struct is_move_only : std::conjunction<neg<std::is_reference<T>>, neg<std::is_copy_constructible<unqualified_t<T>>>, std::is_move_constructible<unqualified_t<T>>> { };
	template <typename T>
	using is_not_move_only = neg<is_move_only<T>>;

	template <typename T>
	struct is_string : std::false_type { };
	template <typename CharT, typename TraitsT, typename AllocT>
	struct is_string<std::basic_string<CharT, TraitsT, AllocT>> : std::true_type { };
	//template <typename T>
	//constexpr inline bool is_string_v = is_string<T>::value;
	template<class T>
    concept is_string_c = is_string<unqualified_t<T>>::value;


	template <typename T>
	struct is_string_view : std::false_type { };
	template <typename CharT, typename TraitsT>
	struct is_string_view<std::basic_string_view<CharT, TraitsT>> : std::true_type { };
	//template <typename T>
	//constexpr inline bool is_string_view_v = is_string_view<T>::value;
	template<class T>
    concept is_string_view_c = is_string_view<unqualified_t<T>>::value;

	template<class T>
    concept is_char_c = any_same<unqualified_t<T>, 
		char, 
	#if defined(__cpp_lib_char8_t)
		char8_t, 
	#endif
		char16_t, char32_t, wchar_t>::value;

	template<class T>
	concept is_arithmetic_c = std::is_arithmetic<T>::value;

	template<class T>
	concept is_c_str_c = is_c_str_of<unqualified_t<T>, char>::value || 
	#if defined(__cpp_lib_char8_t)
		is_c_str_of<unqualified_t<T>, char8_t>::value || 
	#endif
		is_c_str_of<unqualified_t<T>, char16_t>::value || is_c_str_of<unqualified_t<T>, char32_t>::value || is_c_str_of<unqualified_t<T>, wchar_t>::value;

	template<class T>
	concept is_string_like_or_constructible_c = is_string_like_or_constructible<unqualified_t<T>>::value;
}