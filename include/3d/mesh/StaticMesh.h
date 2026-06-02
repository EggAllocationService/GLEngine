//
// Created by Kyle Smith on 2025-10-21.
//
#pragma once

#include <fstream>
#include <vector>
#include "engine_GLUT.h"

#include "glengine_export.h"
#include "Resource.h"
#include "Vectors.h"
#include "pipeline/wgpu/WGPURenderer.h"

namespace glengine::world::mesh {
    struct GLENGINE_EXPORT alignas(sizeof(float4)) Material {
        float4 Diffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
        float4 Specular = float4(0, 0, 0, 1.0f);
        float4 Emissive = float4(0, 0, 0, 1);
        float Shininess = 0.0f;
    };
    /// A StaticMesh is a non-animated mesh
    class GLENGINE_EXPORT StaticMesh : public Resource {
    public:
        StaticMesh(std::istream& file, pipeline::wgpu::WGPURenderer* renderer);

        std::vector<pipeline::wgpu::Vertex> vertices_;
        std::vector<int3> faces_;
        bool hasTexCoords_ = false;

        std::shared_ptr<pipeline::wgpu::GPUMesh> mesh;

    private:
        /// Recalculates all normal vectors for the mesh
        void RecalculateNormals();

        /// Normalizes the mesh so all vertices are within (-1, -1, -1) to (1, 1, 1)
        /// This is to ensure meshes aren't gigantic or tiny for no reason
        void normalize();

        bool hasNormals_ = false;
    };
}
