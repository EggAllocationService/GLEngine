//
// Created by Kyle Smith on 2025-09-29.
//
#pragma once
#include "Matrix.h"

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
}