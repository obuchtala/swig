#include <iostream>
#include "example.h"

#define M_PI 3.14159

Circle::Circle(): radius(1.0) {}

Circle::Circle(double r): radius(r) {
    std::cout << "created Circle with r=" << radius << std::endl;
    std::cout << "ctor for " << (long) this << std::endl;
}

double Circle::area() {
    std::cout << "Circle::area called, r=" << radius << std::endl;
    std::cout << "calling on " << (long) this << std::endl;
    return M_PI*radius*radius;
}
