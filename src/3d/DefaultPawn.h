//
// Created by Kyle Smith on 2025-10-18.
//
#pragma once
#include "3d/Pawn.h"

namespace glengine::world {
    /// Default flying-controlled pawn so we have something to spawn & possess as a fallback
    /// Just a flying camera, basically.
    /// WASD movement controls, mouse look controls
    /// Also supports looking around with the arrow keys, in case the mouse isn't captured
    class DefaultPawn : public Pawn {
    public:
        void Update(double deltaTime) override {};

        void OnPossess(input::InputManager* manager) override;

        void OnUnpossess() override;
    };
}