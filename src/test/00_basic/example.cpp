#include <UXName/UXName.hpp>

#include <iostream>

using namespace Ubpa;

namespace {
	struct A {};
}
enum class B {};
int main() {
	{
		constexpr auto rname = UXName::raw_type_name<const volatile A* const&>().name;
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
}
