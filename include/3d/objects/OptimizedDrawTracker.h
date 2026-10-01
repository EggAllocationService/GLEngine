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

    class GLENGINE_EXPORT OptimizedDrawTracker : public pipeline::RenderObject {
    public:
        OptimizedDrawTracker();
        void UpdateEnd(double deltaTime) override;
        void RenderStart(pipeline::wgpu::RenderBundle &bundle) override;

        void Draw(const std::shared_ptr<mesh::OptimizedMesh>& mesh, OptimizedMeshInstance& instance);
    private:
        std::unordered_map<int, OptimizedInstanceTracker> trackers;
        std::shared_ptr<pipeline::wgpu::MeshPipeline> pipeline;
    };
}
