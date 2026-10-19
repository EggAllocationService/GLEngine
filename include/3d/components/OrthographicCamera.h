//
// Created by kyle on 2026-10-19.
//

#pragma once
#include "CameraLike.h"

namespace glengine::world::components {
    /// Orthographic camera
    class OrthographicCamera : public CameraLike {
    public:
        mat4 GLENGINE_EXPORT GetProjectionMatrix() override;

        float Near = 0.01f;
        float Far = 100.0f;

        float Left = 5.0f;
        float Right = 5.0f;
        float Top = 5.0f;
        float Bottom = 5.0f;
    };
}