module;
#include <iostream>
export module specific;

export struct Specific
{
    static void f() { std::cout << "f() called\n"; }
};
