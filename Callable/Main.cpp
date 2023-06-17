#include <iostream>

#include "Invoker.hpp"

Invoker< void, int&& > inv;

int main()
{
    int val = 10;
    inv( std::move( val ) );
}