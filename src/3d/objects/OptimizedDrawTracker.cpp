//
// Created by Kyle Smith on 2026-10-01.
//

#include "3d/objects/OptimizedDrawTracker.h"
#include "Shaders.h"

namespace glengine::world::objects {
    OptimizedDrawTracker::OptimizedDrawTracker() {
        auto shaders = GetRenderer()->CompileShader(embed_OptimizedMesh_wgsl);

        auto group1 = new WGPUBindGroupLayoutEntry[3];
        for (int i = 0; i < 3; ++i) {
            group1[i] = WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;
            group1[i].binding = i;
            group1[i].buffer = {
                .nextInChain = nullptr,
                .type =WGPUBufferBindingType_ReadOnlyStorage ,
                .hasDynamicOffset = false,
                .minBindingSize = 0
            };
            group1[i].visibility = WGPUShaderStage_Task | WGPUShaderStage_Mesh;
        }

        auto group2 = WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;
        group2.buffer = {
            .nextInChain = nullptr,
            .type = WGPUBufferBindingType_ReadOnlyStorage,
            .hasDynamicOffset = false,
            .minBindingSize = 0
        };
        group2.visibility = WGPUShaderStage_Task | WGPUShaderStage_Mesh;

        auto groups = new WGPUBindGroupLayoutDescriptor[2] {WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT, WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT};
        groups[0].entries = group1;
        groups[0].entryCount = 3;
        groups[1].entries = &group2;
        groups[1].entryCount = 1;

        pipeline = GetRenderer()->BuildMeshPipeline(
            "BuiltinOptimizedMesh",
            shaders,
            true,
            std::span(groups, 2),
            0,
            nullptr
        );

        delete[] groups;
        delete[] group1;
    }

    void OptimizedDrawTracker::UpdateEnd(double deltaTime) {
    }

    void OptimizedDrawTracker::RenderStart(pipeline::wgpu::RenderBundle &bundle) {
    }

    std::shared_ptr<pipeline::wgpu::MeshPipeline> OptimizedDrawTracker::GetPipeline() const {
        return pipeline;
    }
}
