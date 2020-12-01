#ifndef UBPA_UXNAME_HPP
#define UBPA_UXNAME_HPP

#define UXNAME_VERSION_MAJOR 0
#define UXNAME_VERSION_MINOR 0
#define UXNAME_VERSION_PATCH 1

#include <array>
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <iosfwd>
#include <iterator>
#include <limits>
#include <type_traits>
#include <utility>
#include <tuple>

#if !defined(UXNAME_USING_ALIAS_STRING)
#include <string>
#endif
#if !defined(UXNAME_USING_ALIAS_STRING_VIEW)
#include <string_view>
#endif

#if __has_include(<cxxabi.h>)
#include <cxxabi.h>
#include <cstdlib>
#endif

#if defined(__clang__)
#  pragma clang diagnostic push
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wstringop-overflow" // Missing terminating nul 'enum_name_v'.
#elif defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable : 26495) // Variable 'cstring<N>::chars_' is uninitialized.
#endif

// Checks UXName_type compiler compatibility.
#if defined(__clang__) && __clang_major__ >= 5 || defined(__GNUC__) && __GNUC__ >= 7 || defined(_MSC_VER) && _MSC_VER >= 1910
#  undef  UXNAME_TYPE_SUPPORTED
#  define UXNAME_TYPE_SUPPORTED 1
#endif

// Checks UXName_type_rtti compiler compatibility.
#if defined(__clang__)
#  if __has_feature(cxx_rtti)
#    undef  UXNAME_TYPE_RTTI_SUPPORTED
#    define UXNAME_TYPE_RTTI_SUPPORTED 1
#  endif
#elif defined(__GNUC__)
#  if defined(__GXX_RTTI)
#    undef  UXNAME_TYPE_RTTI_SUPPORTED
#    define UXNAME_TYPE_RTTI_SUPPORTED 1
#  endif
#elif defined(_MSC_VER)
#  if defined(_CPPRTTI)
#    undef  UXNAME_TYPE_RTTI_SUPPORTED
#    define UXNAME_TYPE_RTTI_SUPPORTED 1
#  endif
#endif

// Checks UXName_enum compiler compatibility.
#if defined(__clang__) && __clang_major__ >= 5 || defined(__GNUC__) && __GNUC__ >= 9 || defined(_MSC_VER) && _MSC_VER >= 1910
#  undef  UXNAME_ENUM_SUPPORTED
#  define UXNAME_ENUM_SUPPORTED 1
#endif

// Checks UXName_enum compiler aliases compatibility.
#if defined(__clang__) && __clang_major__ >= 5 || defined(__GNUC__) && __GNUC__ >= 9 || defined(_MSC_VER) && _MSC_VER >= 1920
#  undef  UXNAME_ENUM_SUPPORTED_ALIASES
#  define UXNAME_ENUM_SUPPORTED_ALIASES 1
#endif

// Enum value must be greater or equals than UXNAME_ENUM_RANGE_MIN. By default UXNAME_ENUM_RANGE_MIN = -128.
// If need another min range for all enum types by default, redefine the macro UXNAME_ENUM_RANGE_MIN.
#if !defined(UXNAME_ENUM_RANGE_MIN)
#  define UXNAME_ENUM_RANGE_MIN -128
#endif

// Enum value must be less or equals than UXNAME_ENUM_RANGE_MAX. By default UXNAME_ENUM_RANGE_MAX = 128.
// If need another max range for all enum types by default, redefine the macro UXNAME_ENUM_RANGE_MAX.
#if !defined(UXNAME_ENUM_RANGE_MAX)
#  define UXNAME_ENUM_RANGE_MAX 128
#endif

#ifndef UBPA_TSTR
#define UBPA_TSTR

#include <utility>
#include <string_view>

namespace Ubpa {
	template<typename Char, Char... chars>
	struct TStr;
}

namespace Ubpa::detail {
	template<typename STR>
	struct TSTRSizeof;
	template<typename Char>
	struct TSTRSizeof<std::basic_string_view<Char>> {
		static constexpr size_t get(const std::basic_string_view<Char>& str) noexcept {
			return str.size();
		}
	};
	template<typename Char>
	struct TSTRSizeof<const std::basic_string_view<Char>&> : TSTRSizeof<std::basic_string_view<Char>> {};
	template<typename Char>
	struct TSTRSizeof<const std::basic_string_view<Char>> : TSTRSizeof<std::basic_string_view<Char>> {};

	template<typename Char, size_t N>
	struct TSTRSizeof<const Char(&)[N]> {
		static constexpr size_t get(const Char(&str)[N]) noexcept {
			return N - 1;
		}
	};

	template <typename Char, typename T, size_t ...N>
	constexpr decltype(auto) TSTRHelperImpl(std::index_sequence<N...>) {
		return TStr<Char, T::get()[N]...>{};
	}

	template <typename T>
	constexpr decltype(auto) TSTRHelper(T) {
		using Char = std::decay_t<decltype(T::get()[0])>;
		return TSTRHelperImpl<Char, T>(std::make_index_sequence<TSTRSizeof<decltype(T::get())>::get(T::get())>());
	}
}

// [C-style string type (value)]
// in C++20, we can easily put a string into template parameter list
// but in C++17, we just can use this disgusting trick
#define TSTR(s)                                                           \
(Ubpa::detail::TSTRHelper([] {                                            \
    struct tmp { static constexpr decltype(auto) get() { return (s); } }; \
    return tmp{};                                                         \
}()))

namespace Ubpa {
	template<typename Char_, Char_... chars>
	struct TStr {
		using Tag = TStr;
		using Char = Char_;
		template<typename T>
		static constexpr bool NameIs(T = {}) noexcept { return std::is_same_v<T, Tag>; }
		static constexpr char name_data[]{ chars...,Char(0) };
		static constexpr std::basic_string_view<Char> name{ name_data };
	};

	template<typename T>
	struct IsTStr : std::false_type {};

	template<typename Char, Char... chars>
	struct IsTStr<TStr<Char, chars...>> : std::true_type {};
}

#endif // UBPA_TSTR

namespace Ubpa::UXName::details {
#if defined(_MSC_VER)
	template <typename T>
	struct identity {
		using type = T;
	};
#else
	template <typename T>
	using identity = T;
#endif

	template<typename T>
	constexpr auto func_signature_impl() noexcept {
#  if defined(__clang__)
		return std::string_view{ __PRETTY_FUNCTION__ };
#  elif defined(__GNUC__)
		return std::string_view{ __PRETTY_FUNCTION__ };
#  elif defined(_MSC_VER)
		return std::string_view{ __FUNCSIG__ };
#  endif
	}

	template<typename T>
	constexpr auto func_signature()noexcept {
		return TSTR(func_signature_impl<identity<T>>());
	}

	//
	// TStr Utils
	///////////////

	template<typename Str0, typename Str1>
	struct concat_helper;
	template<typename Str0, typename Str1>
	using concat_helper_t = typename concat_helper<Str0, Str1>::type;
	template<typename Char, Char... c0, Char... c1>
	struct concat_helper<TStr<Char, c0...>, TStr<Char, c1...>> {
		using type = TStr<Char, c0..., c1...>;
	};

	template<typename Str0, typename Str1>
	constexpr auto concat(Str0 = {}, Str1 = {}) noexcept {
		return typename concat_helper<Str0, Str1>::type{};
	}

	template<typename Separator, typename... Strs>
	struct concat_seq_helper;
	template<typename Separator, typename... Strs>
	using concat_seq_helper_t = typename concat_seq_helper<Separator, Strs...>::type;
	template<typename Separator>
	struct concat_seq_helper<Separator> {
		using type = TStr<typename Separator::Char>;
	};
	template<typename Separator, typename Str>
	struct concat_seq_helper<Separator, Str> {
		using type = Str;
	};
	template<typename Separator, typename Str, typename... Strs>
	struct concat_seq_helper<Separator, Str, Strs...> {
		using type = concat_helper_t<concat_helper_t<Str, Separator>, concat_seq_helper_t<Separator, Strs...>>;
	};

	template<typename Separator, typename... Strs>
	constexpr auto concat_seq(Separator, Strs...) noexcept {
		return concat_seq_helper_t<Separator, Strs...>{};
	}

	template<typename Str, typename X>
	constexpr size_t find(Str = {}, X = {}) noexcept {
		static_assert(IsTStr<Str>::value && IsTStr<X>::value);
		if constexpr (Str::name.size() >= X::name.size()) {
			for (size_t i = 0; i < Str::name.size() - X::name.size() + 1; i++) {
				bool flag = true;
				for (size_t k = 0; k < X::name.size(); k++) {
					if (Str::name[i + k] != X::name[k]) {
						flag = false;
						break;
					}
				}
				if (flag)
					return i;
			}
		}
		return static_cast<size_t>(-1);
	}

	template<typename Str, typename X>
	constexpr bool starts_with(Str = {}, X = {}) noexcept {
		static_assert(IsTStr<Str>::value && IsTStr<X>::value);
		if (Str::name.size() < X::name.size())
			return false;
		for (size_t i = 0; i < X::name.size(); i++) {
			if (Str::name[i] != X::name[i])
				return false;
		}
		return true;
	}

	template<typename Str, typename X>
	constexpr bool ends_with(Str = {}, X = {}) noexcept {
		static_assert(IsTStr<Str>::value && IsTStr<X>::value);
		if (Str::name.size() < X::name.size())
			return false;
		for (size_t i = 0; i < X::name.size(); i++) {
			if (Str::name[Str::name.size() - X::name.size() + i] != X::name[i])
				return false;
		}
		return true;
	}

	template<size_t N, typename Str>
	constexpr auto remove_prefix(Str = {}) {
		static_assert(IsTStr<Str>::value);
		if constexpr (Str::name.size() >= N)
			return TSTR(decltype(Str::name){Str::name.data() + N});
		else
			return TSTR("");
	}

	template<typename Str, typename X>
	constexpr auto remove_prefix(Str = {}, X = {}) {
		static_assert(IsTStr<Str>::value);
		static_assert(IsTStr<X>::value);
		if constexpr (starts_with<Str, X>())
			return remove_prefix<X::name.size(), Str>();
		else
			return Str{};
	}

	template<size_t N, typename Str>
	constexpr auto remove_suffix(Str = {}) {
		static_assert(IsTStr<Str>::value);
		if constexpr (Str::name.size() >= N)
			return TSTR((decltype(Str::name){Str::name.data(), Str::name.size() - N}));
		else
			return TSTR("");
	}

	template<typename Str, typename X>
	constexpr auto remove_suffix(Str = {}, X = {}) {
		static_assert(IsTStr<Str>::value);
		if constexpr (ends_with<Str, X>())
			return remove_suffix<X::name.size(), Str>();
		else
			return Str{};
	}

	template<size_t N, typename Str>
	constexpr auto get_prefix(Str = {}) {
		static_assert(IsTStr<Str>::value);
		if constexpr (Str::name.size() >= N)
			return TSTR((decltype(Str::name){Str::name.data(), N}));
		else
			return Str{};
	}

	template<size_t N, typename Str>
	constexpr auto get_suffix(Str = {}) {
		static_assert(IsTStr<Str>::value);
		if constexpr (Str::name.size() >= N)
			return TSTR(decltype(Str::name){Str::name.data() + Str::name.size() - N});
		else
			return Str{};
	}

	// [Left, Right)
	template<size_t Idx, size_t Cnt, typename Str, typename X>
	constexpr auto replace(Str = {}, X = {}) {
		static_assert(IsTStr<Str>::value);
		static_assert(IsTStr<X>::value);
		constexpr auto prefix = remove_suffix<Str::name.size() - Idx>(Str{});
		constexpr auto suffix = remove_prefix<Idx + Cnt>(Str{});

		return concat(concat(prefix, X{}), suffix);
	}

	template<typename Str, typename From, typename To>
	constexpr auto replace(Str = {}, From = {}, To = {}) {
		static_assert(IsTStr<Str>::value);
		static_assert(IsTStr<From>::value);
		static_assert(IsTStr<To>::value);
		constexpr size_t idx = find(Str{}, From{});
		if constexpr (idx != static_cast<size_t>(-1))
			return replace(replace<idx, From::name.size()>(Str{}, To{}), From{}, To{});
		else
			return Str{};
	}

	template<typename Str, typename X>
	constexpr auto remove(Str = {}, X = {}) {
		return replace(Str{}, X{}, TSTR(""));
	}

	template<size_t Idx, size_t Cnt, typename Str>
	constexpr auto substr(Str = {}) {
		return get_prefix<Cnt>(remove_prefix<Idx, Str>());
	}

	//
	// Misc
	/////////

	template<typename Str>
	constexpr auto remove_class_key(Str = {}) {
#if defined(__clang__)
		return Str{};
#elif defined(__GNUC__)
		return Str{};
#elif defined(_MSC_VER)
		constexpr auto name1 = details::remove_prefix(Str{}, TSTR("struct "));
		constexpr auto name2 = details::remove_prefix(name1, TSTR("enum "));
		constexpr auto name3 = details::remove_prefix(name2, TSTR("class "));
		return name3;
#endif
	}

	template<typename Str>
	constexpr auto remove_useless_space(Str = {}) {
		constexpr auto t0 = replace(Str{}, TSTR(" *"), TSTR("*"));
		constexpr auto t1 = replace(t0, TSTR("* "), TSTR("*"));
		constexpr auto t2 = replace(t1, TSTR(" &"), TSTR("&"));
		constexpr auto t3 = replace(t2, TSTR("& "), TSTR("&"));
		constexpr auto t4 = replace(t3, TSTR(", "), TSTR(","));
		constexpr auto t5 = replace(t4, TSTR(" >"), TSTR(">"));

		return remove_suffix(t5, TSTR(" "));
	}

	template<typename Str>
	constexpr auto normalize_anonymous_namespace(Str = {}) {
#if defined(__clang__)
		return replace(Str{}, TSTR("{anonymous namespace}"), TSTR("{anonymous}"));
#elif defined(__GNUC__)
		return Str{};
#elif defined(_MSC_VER)
		return replace(Str{}, TSTR("`anonymous namespace'"), TSTR("{anonymous}"));
#endif
	}

	template<typename Str>
	constexpr auto move_cv(Str = {}) {
#if defined(__clang__)
		return Str{};
#elif defined(__GNUC__)
		return Str{};
#elif defined(_MSC_VER)
		if constexpr (ends_with(Str{}, TSTR("*const&")))
			return concat(move_cv(remove_suffix<7, Str>()), TSTR("*const&"));
		else if constexpr (ends_with(Str{}, TSTR("*const&&")))
			return concat(move_cv(remove_suffix<7, Str>()), TSTR("*const&&"));
		else if constexpr (ends_with(Str{}, TSTR("*")))
			return concat(move_cv(remove_suffix<1, Str>()), TSTR("*"));
		else if constexpr (ends_with(Str{}, TSTR("&&")))
			return concat(move_cv(remove_suffix<2, Str>()), TSTR("&&"));
		else if constexpr (ends_with(Str{}, TSTR("&")))
			return concat(move_cv(remove_suffix<1, Str>()), TSTR("&"));
		else {
			if constexpr (ends_with(Str{}, TSTR(" const volatile")))
				return concat(TSTR("const volatile "), move_cv(remove_suffix<sizeof("const volatile"), Str>()));
			else if constexpr (ends_with(Str{}, TSTR(" volatile")))
				return concat(TSTR("volatile "), move_cv(remove_suffix<sizeof("volatile"), Str>()));
			else if constexpr (ends_with(Str{}, TSTR(" const")))
				return concat(TSTR("const "), move_cv(remove_suffix<sizeof("const"), Str>()));
			else
				return Str{};
		}
#endif
	}

	template<typename Str>
	constexpr size_t get_template_arg_num(Str = {}) {
		size_t k = 0;
		size_t rst = 0;
		for (size_t i = 0; i < Str::name.size(); i++) {
			if (Str::name[i] == '<') {
				if (k == 0 && Str::name[i + 1] != '>')
					++rst;
				++k;
			}
			else if (Str::name[i] == ',') {
				if (k == 1)
					++rst;
			}
			else if (Str::name[i] == '>')
				k--;
		}
		return rst;
	}

	template<typename Str, size_t N>
	constexpr auto get_template_arg_index(Str = {}) {
		std::array<size_t, N + 1> indices = { static_cast<size_t>(-1) };
		size_t k = 0;
		size_t cnt = 0;
		for (size_t i = 0; i < Str::name.size(); i++) {
			if (Str::name[i] == '<') {
				if (k == 0 && Str::name[i + 1] != '>') {
					indices[cnt] = i + 1;
					++cnt;
				}
				++k;
			}
			else if (Str::name[i] == ',') {
				if (k == 1) {
					indices[cnt] = i + 1;
					++cnt;
				}
			}
			else if (Str::name[i] == '>') {
				k--;
				if (k == 0)
					indices[cnt] = i + 1;
			}
		}
		return indices;
	}
	template<typename Str>
	constexpr auto normalize_type_name(Str = {});

	template<typename Str, size_t... Ns>
	constexpr auto normalize_template_arg_type_name(Str = {}, std::index_sequence<Ns...> = {}) {
		constexpr size_t N = sizeof...(Ns);
		constexpr auto seperator = TSTR(",");
		constexpr auto indices = get_template_arg_index<Str, N>();
		constexpr auto template_args = std::tuple{ normalize_type_name(substr<indices[Ns], indices[Ns + 1] - indices[Ns] - 1, Str>()) ... };
		constexpr auto template_args_name =
			std::apply([seperator](auto... args) {
			return concat_seq(seperator, args...);
				}, template_args);
		return concat(concat(get_prefix<indices[0], Str>(), template_args_name), remove_prefix<indices[N] - 1, Str>());
	}

	template<typename Str>
	constexpr auto normalize_type_name(Str) {
		constexpr auto name0 = remove_class_key(Str{});
		constexpr auto name1 = move_cv(name0);
		constexpr size_t N = get_template_arg_num(name1);
		if constexpr (N > 0) {
			return normalize_template_arg_type_name(name1, std::make_index_sequence<N>{});;
		}
		else
			return name1;
	}
}

namespace Ubpa::UXName {
	template<typename T>
	constexpr auto raw_type_name()noexcept {
		constexpr auto sig = details::func_signature<T>();
#if defined(UXNAME_TYPE_SUPPORTED) && UXNAME_TYPE_SUPPORTED
#  if defined(__clang__)
		return details::remove_suffix<1>(details::remove_prefix<55>(sig));
#  elif defined(__GNUC__)
		return details::remove_suffix<1>(details::remove_prefix<70>(sig));
#  elif defined(_MSC_VER)
		return details::remove_suffix<17>(details::remove_prefix<95>(sig));
#  endif
#else
		return TSTR(""); // Unsupported compiler.
#endif
	}

	template<typename T>
	constexpr auto type_name()noexcept {
		constexpr auto sig = details::func_signature<T>();
#if defined(UXNAME_TYPE_SUPPORTED) && UXNAME_TYPE_SUPPORTED
#  if defined(__clang__)
		constexpr auto name = details::remove_suffix<1>(details::remove_prefix<55>(sig));
#  elif defined(__GNUC__)
		constexpr auto name = details::remove_suffix<1>(details::remove_prefix<70>(sig));
#  elif defined(_MSC_VER)
		constexpr auto name = details::remove_suffix<17>(details::remove_prefix<95>(sig));
#  endif
		constexpr auto name1 = details::normalize_anonymous_namespace(name);
		constexpr auto name2 = details::remove_useless_space(name1);
		constexpr auto name3 = details::normalize_type_name(name2);
		return name3;
#else
		return TSTR(""); // Unsupported compiler.
#endif
	}
}

#if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#  pragma warning(pop)
#endif

#endif // UBPA_UXNAME_HPP
