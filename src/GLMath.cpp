#include "GLMath.h"
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

std::unique_ptr<std::vector<float3>> glengine::math::subdividePolygon(std::span<const float3> polygon, int additionalVertices)
{
    // find out the total perimeter length of the polygon
    double perimeter = 0;
    for (int i = 0; i < polygon.size(); i++) {
        perimeter += (float2(polygon[i].xy) - polygon[(i + 1) % polygon.size()].xy).len();
    }
    
    // add evenly spaced points along the polygon
    double distanceBetweenAddedPoints = perimeter / additionalVertices;
    auto newPolygon = std::make_unique<std::vector<float3>>();

    double remainingLength = distanceBetweenAddedPoints;
    // for each line segment in the polygon
    for (int i = 0; i < polygon.size(); i++) {
        float2 start = float2(polygon[i].xy);
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
        // edge case for floating-point inaccuracy
        newPolygon->push_back(polygon[0]);
    }

    return newPolygon;
}
