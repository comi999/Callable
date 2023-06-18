#include <iostream>

#include "Invoker.hpp"

int main()
{
	std::vector< int > vals = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0 };
	Invoker< int > inv = [ vals = std::move( vals ) ]() { return vals[ 2 ]; };

	int res = inv();
	inv.Unbind();
}