//
// Created by kyle on 2026-10-19.
//
#pragma once
#include "Matrix.h"
#include "3d/ActorPrimitiveComponent.h"

namespace glengine::world::components {
    /// Represents a viewpoint into the world
    class CameraLike : public ActorPrimitiveComponent {
    public:
        ~CameraLike() override = default;

        virtual mat4 GetProjectionMatrix() = 0;
    };
}
