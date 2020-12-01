#include <UXName/UXName.hpp>

#include <iostream>

using namespace Ubpa;

struct A {};
int main() {
	constexpr auto name = UXName::raw_type_name<A>().name;
	std::cout << name << std::endl;
}
