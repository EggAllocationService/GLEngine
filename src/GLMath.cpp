//
// Created by Kyle Smith on 2025-09-29.
//
#include "GLMath.h"
#include <cmath>

mat3 glengine::math::rotation2D(float theta) {
    float trigValues[9] = {cos(theta), sin(theta), 0, -sin(theta), cos(theta), 0, 0, 0, 1};
    return mat3(trigValues);
}

mat3 glengine::math::scale2D(float2 scale) {
    mat3 result = mat3::identity();

    result[0]->set(0, scale.x);
    result[1]->set(1, scale.y);

    return result;
}

mat3 glengine::math::translate2D(float2 translation) {
    mat3 result = mat3::identity();

    result[2]->set(0, translation.x);
    result[2]->set(1, translation.y);

    return result;
}