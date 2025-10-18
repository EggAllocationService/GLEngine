//
// Created by Kyle Smith on 2025-10-18.
//

#include "3d/components/CameraComponent.h"

#include "Engine.h"
#include "3d/Actor.h"

void glengine::world::components::CameraComponent::SetProjectionMatrix() {
    auto screenSize = float2(GetActor()->GetEngine()->GetWindowSize());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(FoV, screenSize.x / screenSize.y, Near, Far);
    glMatrixMode(GL_MODELVIEW);
}
