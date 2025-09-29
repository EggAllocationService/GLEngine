//
// Created by Kyle Smith on 2025-09-29.
//
#include "GLMath.h"
#include <cmath>

mat3 glengine::math::rotationMatrix(float theta) {
    float trigValues[9] = {cos(theta), sin(theta), 0, -sin(theta), cos(theta), 0, 0, 0, 1};
    return mat3(trigValues);
}
