#include <UXName/UXName.hpp>

#include <iostream>

using namespace Ubpa;

namespace {
	struct A {};
}
enum class B {};
template<typename T, T i, unsigned j, const char* k, typename U>
class C {};
int main() {
	{
		constexpr auto rname = UXName::raw_type_name<const volatile A * const &>().name;
		constexpr auto name = UXName::type_name<const volatile A * const &>().name;
		std::cout << "|" << rname << "|" << std::endl;
		std::cout << "|" << name << "|" << std::endl;
	}
	{
		constexpr auto rname = UXName::raw_type_name<B>().name;
		constexpr auto name = UXName::type_name<B>().name;
		std::cout << "|" << rname << "|" << std::endl;
		std::cout << "|" << name << "|" << std::endl;
	}
	{
		using T = C<int, -2, 3, nullptr, const volatile std::string_view* const&>;
		constexpr auto rname = UXName::raw_type_name<T>().name;
		constexpr auto name = UXName::type_name<T>().name;
		std::cout << "|" << rname << "|" << std::endl;
		std::cout << "|" << name << "|" << std::endl;
	}
}
