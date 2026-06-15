//
// Created by Kyle Smith on 2026-05-29.
//

#pragma once
#include "pipeline/RenderObject.h"
namespace glengine::world::objects {
    namespace instanced {
        struct alignas(16) InstanceData {
            mat4 matrix;
            float4 diffuse;
            float4 ambient;
            float4 specular;
            float shininess;
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

        void Draw(const std::shared_ptr<pipeline::wgpu::GPUMesh>& mesh, const instanced::InstanceData &data);

    private:
        std::unordered_map<int, instanced::InstanceTracker> buffers;
        std::shared_ptr<pipeline::wgpu::RenderPipeline> pipeline;
        pipeline::wgpu::WGPURenderer* renderer;
    };
}