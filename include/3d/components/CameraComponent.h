//
// Created by Kyle Smith on 2025-10-18.
//
#pragma once
#include "3d/ActorPrimitiveComponent.h"

namespace glengine::world::components {
    /// Represents a viewpoint into the world
    class CameraComponent : public ActorPrimitiveComponent {
    public:
        void Update(double) override {}

        void SetProjectionMatrix();

        float FoV = 90.0;
        float Near = 0.1;
        float Far = 100.0;
    };
}
