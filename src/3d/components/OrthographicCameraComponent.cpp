//
// Created by kyle on 2026-10-19.
//

#include "3d/components/OrthographicCameraComponent.h"

#include "Engine.h"
#include "3d/Actor.h"
#include "GLMath.h"


namespace glengine::world::components {
    mat4 OrthographicCameraComponent::GetProjectionMatrix() {
        auto screenSize = float2(GetActor()->GetEngine()->GetWindowSize());
        float aspect = screenSize.y / screenSize.x;

        return math::orthographicMatrix(-Size / aspect, Size / aspect, -Size, Size, Near, Far);
    }
}