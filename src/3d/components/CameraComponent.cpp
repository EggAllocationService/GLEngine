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

    // invert x axis so that +x is right
    mat4 view = mat4::identity();
    view[0]->set(0, -1);
    glMultMatrixf(static_cast<const float *>(view));

    glMatrixMode(GL_MODELVIEW);
}
