//
// Created by Kyle Smith on 2025-09-26.
//

#include "Matrix.h"
#include <iostream>

int main() {
    mat4 m1 = mat4::identity();
    mat4 m2 = mat4::identity();
    mat4 m3 = m1 * m2;

    std::cout << "Hello, World!" << std::endl;

    std::cout << m1;
    std::cout << m3;
}
