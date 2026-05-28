//
// Created by Kyle Smith on 2025-10-18.
//

#include "3d/components/CameraComponent.h"

#include "Engine.h"
#include "3d/Actor.h"
#include "GLMath.h"
using namespace glengine::world::components;

void CameraComponent::Update(double)
{
    // no updates
}

mat4 CameraComponent::GetProjectionMatrix() {
    auto screenSize = float2(GetActor()->GetEngine()->GetWindowSize());
    float fovRad = FoV * (PI / 180.0);

    return math::perspectiveMatrix(fovRad, screenSize.y / screenSize.x, Near, Far);
}
