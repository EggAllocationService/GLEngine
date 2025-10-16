//
// Created by Kyle Smith on 2025-09-29.
//
#pragma once
#include "Matrix.h"
#include <vector>
#include <span>
#include <memory>

namespace glengine::math {
    /// Creates a 2d clockwise rotation matrix:
    /// cos theta, -sin theta, 0
    /// sin theta, cos theta , 0
    /// 0        , 0         , 1
    /// `theta` is an angle in radians
    mat3 rotation2D(float theta);

    /// Creates a scale matrix:
    /// scale.x, 0      , 0
    /// 0      , scale.y, 0
    /// 0      , 0      , 1
    mat3 scale2D(float2 scale);

    /// Creates a translation matrix:
    /// 1, 0, translation.x
    /// 0, 1, translation.y
    /// 0, 0, 1
    mat3 translate2D(float2 translation);

    /// Linear interpolation, templated so it can be used with vectors
    template<typename T>
    T lerp(T a, T b, float c) {
        return (a * (1 - c)) + (b * c);
    }

    /// Subdivides a polygon, adding `additionalVertices` extra vertices, spaced evenly along the perimeter
    /// As the result will be larger, a newly allocated vector is returned.
    std::unique_ptr<std::vector<float3>> subdividePolygon(std::span<const float3> polygon, int additionalVertices);

}