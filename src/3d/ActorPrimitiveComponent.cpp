//
// Created by Kyle Smith on 2025-10-17.
//

#include "3d/ActorPrimitiveComponent.h"
#include "3d/Actor.h"

float3 glengine::world::ActorPrimitiveComponent::GetAbsolutePosition() const {
    auto selfMatrix = GetTransformMatrix();
    auto parentMatrix = GetActor()->GetTransformMatrix();
    auto resultMatrix = parentMatrix * selfMatrix;

    // 4th column of the resulting matrix represents the origin
    auto resultVec = *resultMatrix[3];

    return {resultVec[0], resultVec[1], resultVec[2]};
}

float3 glengine::world::ActorPrimitiveComponent::GetForwardVector() const {
    vecn<float, 4> vector;
    vector[0] = 0;
    vector[1] = 0;
    vector[2] = 1;
    vector[3] = 1;

    auto result = GetActor()->GetTransformMatrix() *  GetTransformMatrix() * vector;

    return float3(result[0], result[1], result[2]).norm();
}
