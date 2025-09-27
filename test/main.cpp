//
// Created by Kyle Smith on 2025-09-26.
//

#include "Matrix.h"
#include <iostream>

int main() {
    float m1d[6] = {1.0, 4.0, 2.0, 5.0, 3.0, 6.0};
    auto m1 = Matrix<float, 3, 2>(m1d);

    float m2d[6] = {7, 9, 11, 8, 10, 12};
    auto m2 = Matrix<float, 2, 3>(m2d);

    std::cout << "Hello, World!" << std::endl;
    std::cout << m1;
    std::cout << m2;

    auto result = m1 * m2;
    std::cout << result;
}
