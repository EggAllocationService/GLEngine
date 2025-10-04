//
// Created by Kyle Smith on 2025-09-29.
//
#pragma once
#include "Matrix.h"
#include <vector>
#include <span>

namespace glengine::math {
    /**
     * Creates a 2d clockwise rotation matrix by the given angle
     * @param theta angle of rotation in radians
     */
    mat3 rotation2D(float theta);

    /**
     * Creates a 2d scale transform matrix, using the given x/y values
     * @param scale amount to scale each axis by
     */
    mat3 scale2D(float2 scale);

    /**
     * Creates a 2d translate transform matrix, using the given x/y values
     * @param scale amount to translate each axis by
     */
    mat3 translate2D(float2 translation);
    
    template<typename T>
    struct interp {
        T a;
        T b;
        float c;

        explicit operator T() const {
            return lerp(a, b, c);
        }
    };

    template<typename T>
    T lerp(T a, T b, float c) {
        return (a * (1 - c)) + (b * c);
    }

    std::unique_ptr<std::vector<float3>> subdividePolygon(std::span<const float3> polygon, int additionalVertices);

}