//
// Created by Kyle Smith on 2025-10-18.
//
#pragma once
#include "3d/Pawn.h"

namespace glengine::world {
    /// a stationary pawn so that the engine can actually render stuff
    class DefaultPawn : public Pawn {
    public:
        void Update(double deltaTime) override {};
    };

}