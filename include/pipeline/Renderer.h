//
// Created by Kyle Smith on 2025-11-26.
//
#pragma once
#include "Matrix.h"
#include "3d/mesh/StaticMesh.h"

class Renderer {
public:
    virtual ~Renderer() = default;
    virtual void SetMatrices(mat4 viewMatrix, mat4 projectionMatrix) = 0;
    virtual void RenderMesh(const glengine::world::mesh::StaticMesh *mesh, glengine::world::mesh::Material material, mat4 transform) = 0;
};
