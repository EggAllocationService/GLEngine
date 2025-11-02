//
// Created by Kyle Smith on 2025-09-29.
//
#include "GLMath.h"
#include <cmath>

mat3 glengine::math::rotation2D(const float theta) {
    // creates a matrix that looks like this:
    // cos x, -sin x, 0
    // sin x, cos x , 0
    // 0    , 0     , 1
    // basically a standard 2d rotation matrix but 3x3 so it fits on my 2d matrix stack

    return {
        {cosf(theta), sinf(theta), 0, -sinf(theta), cosf(theta), 0, 0, 0, 1}
    };
}

mat3 glengine::math::scale2D(const float2 scale) {
    // creates a scale matrix:
    // scale.x, 0      , 0
    // 0      , scale.y, 0
    // 0      , 0      , 1
    mat3 result = mat3::identity();

    result[0]->set(0, scale.x);
    result[1]->set(1, scale.y);

    return result;
}

mat3 glengine::math::translate2D(const float2 translation) {
    // creates a translation matrix:
    // 1, 0, translation.x
    // 0, 1, translation.y
    // 0, 0, 1
    mat3 result = mat3::identity();

    result[2]->set(0, translation.x);
    result[2]->set(1, translation.y);

    return result;
}

float4 glengine::math::quatFromEuler(float3 angles) {
    /// adapted from https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
    float cx = cosf(angles.x / 2.0);
    float sx = sinf(angles.x / 2.0);
    float cy = cosf(angles.y / 2.0);
    float sy = sinf(angles.y / 2.0);
    float cz = cosf(angles.z / 2.0);
    float sz = sinf(angles.z / 2.0);

    return float4(
        sx*cy*cz - cx*sy*sz,
        cx*sy*cz + sx*cy*sz,
        cx*cy*sz - sx*sy*cz,
        cx*cy*cz + sx*sy*sz
    );
}

std::unique_ptr<std::vector<float3>> glengine::math::subdividePolygon(std::span<const float3> polygon,
                                                                      int additionalVertices) {
    // find the total perimeter of the polygon
    double perimeter = 0;
    for (int i = 0; i < polygon.size(); i++) {
        perimeter += (float2(polygon[i].xy) - polygon[(i + 1) % polygon.size()].xy).len();
    }

    // determine spacing between points
    double distanceBetweenAddedPoints = perimeter / additionalVertices;
    auto newPolygon = std::make_unique<std::vector<float3>>();

    double remainingLength = distanceBetweenAddedPoints;
    // for each line segment in the polygon
    for (int i = 0; i < polygon.size(); i++) {
        float2 start = polygon[i].xy;
        float2 end = polygon[(i + 1) % polygon.size()].xy;
        float2 direction = (end - start).norm();

        // add the starting vertex
        newPolygon->push_back(polygon[i]); // make sure we keep existing vertices

        // take `remainingLength` sized steps along the line segment
        while (true) {
            float distance = (end - start).len();
            if (distance < remainingLength) {
                // line segment is not long enough for the remaining length till next point
                // will have to continue on next line segment
                remainingLength -= distance;
                break;
            }

            // move start vertex towards end by `remainingLength`
            start = start + (direction * remainingLength);

            // add new vertex at `start`
            newPolygon->push_back(float3(start, 1.0));

            // reset remainingLength
            remainingLength = distanceBetweenAddedPoints;
        }
    }

    if (newPolygon->size() == (polygon.size() + additionalVertices) - 1) {
        // edge case: sometimes we don't quite add enough due to floating-point inaccuracy
        // so just add another point at the start.
        newPolygon->push_back(polygon[0]);
    }

    return newPolygon;
}

float dot(float3 a, float3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// these formulas are based on http://perry.cz/articles/ProjectionMatrix.xhtml

mat4 glengine::math::viewMatrix(mat4 cameraTransformMatrix) {
    float3 right = reinterpret_cast<float4*>(cameraTransformMatrix[0])->xyz;
    float3 up = reinterpret_cast<float4*>(cameraTransformMatrix[1])->xyz;
    float3 look = reinterpret_cast<float4*>(cameraTransformMatrix[2])->xyz;
    float3 eye = reinterpret_cast<float4*>(cameraTransformMatrix[3])->xyz;

    float A = -dot(right, eye);
    float B = -dot(up, eye);
    float C = -dot(look, eye);

    return {
        {
            right.x, up.x, look.x, 0,
            right.y, up.y, look.y, 0,
            right.z, up.z, look.z, 0,
            A,       B,    C,      1

        }
    };
}

mat4 glengine::math::perspectiveMatrix(float fov, float aspect, float near, float far) {
    float A = (1.0 / tanf(fov * 0.5));
    float B = aspect / tanf(fov * 0.5);
    float C = (-(far + near)) / (far - near);
    float D = 1.0;
    float E = (2 * far * near) / (far - near);

    return {
        {
            A, 0, 0, 0,
            0, B, 0, 0,
            0, 0, C, D,
            0, 0, E, 0
        }
    };
}