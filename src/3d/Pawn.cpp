//
// Created by Kyle Smith on 2025-10-18.
//

#include "3d/Pawn.h"

#include "Engine.h"

glengine::world::Pawn::Pawn() {
    // have a camera centered on the origin just in case
    activeCamera = CreateComponent<components::CameraComponent>();
}
