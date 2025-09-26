//
// Created by Kyle Smith on 2025-09-26.
//

#ifndef GLENGINE_MATRIX_H
#define GLENGINE_MATRIX_H
#include <string>
#include <iostream>

#include "Vectors.h"

template <typename T, int X, int Y>
struct Matrix {
    // column-major
    T data[X * Y];

    vecn<T, Y>* operator[](int row) {
        return (vecn<T, Y> *) &data[row * X];
    }

    const int2 size = int2(X, Y);

    template<int X2, int Y2> Matrix<T, X, Y2> operator *(Matrix<T, X2, Y2>& other) {
        static_assert(X == Y2, "Matrix multiplication not defined for matrices without matching row counts");
        Matrix<T, X, Y2> r;

        for (int column = 0; column < X; column++) {

            vecn<T, Y2> *cur_column = other[column];
            vecn<T, Y> acc = *(this->operator[](0)) * 0;

            for (int i = 0; i < Y2; i++) {
                acc = acc + (*this->operator[](0) * cur_column[i]);
            }

            for (int i = 0; i < Y; i++) {
                r.data[(column * X) + i] = acc.data[i];
            }
        }

        return r;
    }

    friend std::ostream& operator<<(std::ostream& os, const Matrix &m)
    {
        os << "matrix:\n";

        for (int x = 0; x < X; x++) {
            for (int y = 0; y < Y; y++) {
                os << m.data[x * Y + y] << " ";
            }
            os << "\n";
        }
        return os;
    }

    static Matrix identity() {
        static_assert(X == Y, "Matrix identity not defined for non-square matrices");
        Matrix m;
        for (int i = 0; i < X; i++) {
            for (int j = 0; j < Y; j++) {
                m.data[i * Y + j] = i == j ? 1 : 0;
            }
        }
        return m;
    }
};

typedef Matrix<float, 4, 4> mat4;
typedef Matrix<float, 3, 3> mat3;

#endif //GLENGINE_MATRIX_H