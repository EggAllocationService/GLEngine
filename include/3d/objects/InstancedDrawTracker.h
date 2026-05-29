//
// Created by Kyle Smith on 2026-05-29.
//

#pragma once
#include "pipeline/RenderObject.h"
namespace glengine::world::objects {
    namespace instanced {
        struct InstanceData {
            mat4 matrix;
        };

        struct InstanceTracker {
            std::unique_ptr<pipeline::wgpu::TypedGPUBuffer<InstanceData>> data;
            std::shared_ptr<pipeline::wgpu::RenderPipeline> pipeline;
            std::shared_ptr<pipeline::wgpu::GPUMesh> mesh;
        };
    }

    class InstancedDrawTracker : public pipeline::RenderObject {
    public:
        InstancedDrawTracker();

        void UpdateEnd(double deltaTime) override;

        void RenderStart(pipeline::wgpu::RenderBundle &bundle) override;

        void Draw(std::shared_ptr<pipeline::wgpu::GPUMesh>& mesh, instanced::InstanceData data);

    private:
        std::unordered_map<int, instanced::InstanceTracker> buffers;
        std::shared_ptr<pipeline::wgpu::RenderPipeline> pipeline;
        pipeline::wgpu::WGPURenderer* renderer;
    };
}