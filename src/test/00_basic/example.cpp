#include <UXName/UXName.hpp>

#include <iostream>
#include <vector>

using namespace Ubpa;

template<int, typename T>
struct A {};

template<typename T, int...>
struct AA {};

template<int N, typename T>
struct Ubpa::UXName::details::to_typename_template_type<A<N, T>> {
	template<typename Int, typename T>
	struct A;
	using type = A<std::integral_constant<int, N>, T>;
};
template<typename T, int... Ns>
struct Ubpa::UXName::details::to_typename_template_type<AA<T, Ns...>> {
	template<typename T, typename... Ints>
	struct AA;
	using type = AA<T, std::integral_constant<int, Ns>...>;
};
struct B {
	struct C {};
};

void f(A<2,int>) {
}

template<typename T>
struct C {
	struct D {};
	void f(A<2, int>, A<4, double>) {
	}
	void g(std::vector<int>) {
	}
};

template<auto>
struct E {};

int main() {
	std::is_reference_v<int* const&>;
	std::is_lvalue_reference_v<int* const>;
	{
		auto f = []() {};
		std::cout << "|" << UXName::raw_type_name<E<&C<int>::g>>().name << "|" << std::endl;
		std::cout << "|" << UXName::type_name<decltype(f)>().name << "|" << std::endl;
		std::cout << "|" << UXName::raw_type_name<const A<2, int>* const&>().name << "|" << std::endl;
		std::cout << "|" << UXName::raw_type_name<const A<2, int>* const&>().name << "|" << std::endl;
		std::cout << "|" << UXName::kernel_type_name<A<2, int>>().name << "|" << std::endl;
		std::cout << "|" << UXName::type_name<const A<2, int>*const&>().name << "|" << std::endl;
		std::cout << "|" << UXName::type_name<decltype(&f)>().name << "|" << std::endl;
		std::cout << "|" << UXName::type_name<decltype(&C<float>::f)>().name << "|" << std::endl;
		std::cout << "|" << UXName::type_name<C<decltype(&C<float>::f)>::D>().name << "|" << std::endl;
		std::cout << "|" << UXName::type_name<decltype(&C<A<2, int>>::g)>().name << "|" << std::endl;
		std::cout << "|" << UXName::type_name<A<2, int>[][8]>().name << "|" << std::endl;
		std::cout << "|" << UXName::type_name<const AA<A<2, int>, 2, 4, 5, 6>* const&>().name << "|" << std::endl;
	}
}
