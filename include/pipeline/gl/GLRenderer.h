//
// Created by Kyle Smith on 2025-11-26.
//
#pragma once
#include "pipeline/Renderer.h"

namespace glengine::pipeline::gl {
    class GLENGINE_EXPORT GLRenderer final : public Renderer {
    public:
        GLRenderer();

        void SetMatrices(mat4 viewMatrix, mat4 projectionMatrix) override;
        void RenderMesh(const glengine::world::mesh::StaticMesh *mesh, glengine::world::mesh::Material material, mat4 transform) override;
    };

}