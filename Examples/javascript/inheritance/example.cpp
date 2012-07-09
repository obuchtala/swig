#include "example.h"

#include <iostream>

void A::foo()
{
    std::cout << "A::foo()" << std::endl;
}
   
void A::bar()
{
    std::cout << "A::bar()" << std::endl;
}

void B::bar()
{
    std::cout << "B::bar()" << std::endl;
}
