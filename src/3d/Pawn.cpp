//
// Created by Kyle Smith on 2025-10-18.
//

#include "3d/Pawn.h"

#include "Engine.h"

glengine::world::Pawn::Pawn() {
    defaultCamera = CreateComponent<components::CameraComponent>();
}
