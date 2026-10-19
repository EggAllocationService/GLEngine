//
// Created by kyle on 2026-10-19.
//

#pragma once
#include "CameraLike.h"

namespace glengine::world::components {
    /// Orthographic camera
    class OrthographicCameraComponent : public CameraLike {
    public:
        mat4 GLENGINE_EXPORT GetProjectionMatrix() override;

        float Near = 0.01f;
        float Far = 100.0f;

        // half-height of the view volume
        float Size = 1.0f;
    };
}