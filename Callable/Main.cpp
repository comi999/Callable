#include <iostream>

#include "Invoker.hpp"

int main()
{
	std::vector< int > vals = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0 };
	auto lam = [ vals = std::move( vals ) ]() { return vals[ 2 ]; };
	Invoker< int > inv = lam;
	InvokerHelpers::DestroyIfLambdaStorage( nullptr );
	bool bres = inv == Invoker<int>();

	int res = inv();
	inv.Unbind();
}