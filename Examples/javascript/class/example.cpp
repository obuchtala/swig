#include <iostream>
#include "example.h"

#define M_PI 3.14159

Circle::Circle(): radius(1.0) {}

Circle::Circle(double r): radius(r) {
    std::cout << "created Circle with r=" << radius << std::endl;
}

double Circle::area() {
    std::cout << "Circle::area called, r=" << radius << std::endl;
    return M_PI*radius*radius;
}
