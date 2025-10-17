//
// Created by Kyle Smith on 2025-10-17.
//

#include "3d/ActorPrimitiveComponent.h"

float3 glengine::world::ActorPrimitiveComponent::GetAbsolutePosition() const {
    auto selfMatrix = GetTransformMatrix();
    auto parentMatrix = GetActor()->GetTransformMatrix();
    auto resultMatrix = parentMatrix * selfMatrix;

    auto testVec = vecn<float, 4>::zero();
    testVec[3] = 1.0f;

    auto resultVec = resultMatrix * testVec;

    return float3(resultVec[0], resultVec[1], resultVec[2]);
}
