#include <iostream>

#include "Invoker.hpp"
#include "Delegate.hpp"

struct Struct
{
	int operator()() { return 100; }
};

int main()
{
	Action<int> act0;
	const Action<int> act1;
	act0 = act1;
	Struct s;
	auto inv = std::make_invoker( &s );

	return 0;
}