#include <iostream>

#include "Invoker.hpp"
#include "Delegate.hpp"

int main()
{
	Action<int> act0;
	const Action<int> act1;
	act0 = act1;

	return 0;
}