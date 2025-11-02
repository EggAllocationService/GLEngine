//
// Created by Kyle Smith on 2025-10-18.
//

#include "3d/components/CameraComponent.h"

#include "Engine.h"
#include "3d/Actor.h"
#include "GLMath.h"
using namespace glengine;

void world::components::CameraComponent::SetProjectionMatrix() {
    auto screenSize = float2(GetActor()->GetEngine()->GetWindowSize());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    const auto m = math::perspectiveMatrix(FoV, screenSize.x / screenSize.y, Near, Far);

    glLoadMatrixf(static_cast<const float*>(m));

    glMatrixMode(GL_MODELVIEW);
}
