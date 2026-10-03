//
// Created by Kyle Smith on 2026-10-01.
//
#pragma once
#include "3d/mesh/OptimizedMesh.h"
#include "pipeline/RenderObject.h"

namespace glengine::world::objects {
    struct OptimizedMeshInstance {
        mat4 transform;
    };
    struct OptimizedInstanceTracker {
        std::unique_ptr<pipeline::wgpu::TypedGPUBuffer<OptimizedMeshInstance>> instances;
        std::shared_ptr<mesh::OptimizedMesh> mesh;
        std::shared_ptr<pipeline::wgpu::MeshPipeline> pipeline;
    };

    class OptimizedDrawTracker : public pipeline::RenderObject {
    public:
        GLENGINE_EXPORT OptimizedDrawTracker();
        GLENGINE_EXPORT void UpdateEnd(double deltaTime) override;
        GLENGINE_EXPORT void RenderStart(pipeline::wgpu::RenderBundle &bundle) override;

        GLENGINE_EXPORT void Draw(const std::shared_ptr<mesh::OptimizedMesh>& mesh, OptimizedMeshInstance& instance);
    private:
        std::unordered_map<int, OptimizedInstanceTracker> trackers;
        std::shared_ptr<pipeline::wgpu::MeshPipeline> pipeline;
    };
}
