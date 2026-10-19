//
// Created by Kyle Smith on 2025-10-18.
//
#pragma once
#include "CameraLike.h"
#include "glengine_export.h"

namespace glengine::world::components {
    /// Represents a viewpoint into the world
    class GLENGINE_EXPORT CameraComponent : public CameraLike {
    public:
        void Update(double) override;

        // Sets the GL_PROJECTION matrix
        mat4 GetProjectionMatrix() override;

        // Field of View in degrees
        float FoV = 90.0f;

        // Near plane
        float Near = 0.1f;
        
        // Far plane
        float Far = 100.0f;
    };
}
