#include <iostream>
#include <array>
#include <vector>
#include "Invoker.hpp"
#include "Delegate.hpp"

int main()
{
	std::vector< int > v = { 1, 2, 3, 4 };

	Invoker< int, int& > inv0;
	Invoker< int, int& > inv1 = [ v = v ]( int& a ) mutable { int c = v.size(); v.clear(); return c; };

	inv0 = inv1;

	int a = 1;
	int res0 = inv0( a );
	int res1 = inv1( a );
}