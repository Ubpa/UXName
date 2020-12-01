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

	template<typename TStr, typename TX>
	constexpr bool starts_with(TStr = {}, TX = {}) noexcept {
		static_assert(IsTStr<TStr>::value && IsTStr<TX>::value);
		if (TStr::name.size() < TX::name.size())
			return false;
		for (size_t i = 0; i < TX::name.size(); i++) {
			if (TStr::name[i] != TX::name[i])
				return false;
		}
		return true;
	}

	template<size_t N, typename TStr>
	constexpr auto remove_prefix(TStr = {}) {
		static_assert(IsTStr<TStr>::value);
		if constexpr (TStr::name.size() >= N)
			return TSTR(decltype(TStr::name){TStr::name.data() + N});
		else
			return TSTR("");
	}

	template<size_t N, typename TStr>
	constexpr auto remove_suffix(TStr = {}) {
		static_assert(IsTStr<TStr>::value);
		if constexpr (TStr::name.size() >= N)
			return TSTR((decltype(TStr::name){TStr::name.data(), TStr::name.size() - N}));
		else
			return TSTR("");
	}

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
}

#if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#  pragma warning(pop)
#endif

#endif // UBPA_UXNAME_HPP
