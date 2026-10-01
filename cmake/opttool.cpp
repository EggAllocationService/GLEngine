//
// Created by Kyle Smith on 2026-10-01.
//


#include <iostream>
#include <ostream>
#include <fstream>
#include "optimizer.h"

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "Usage: ./opttool <source> <destination>" << std::endl;
        return 1;
    }
    auto result = optimizeMesh(argv[1]);

    std::ofstream output(argv[2], std::ios::out | std::ios::trunc);

    output.write(result.data(), result.size());

    output.flush();
    output.close();
}
