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
        auto session = GetRenderer()->GetTransferManager()->CreateSession("Optimized Draw Data", 0);
        for (auto& x : trackers) {
            auto& tracker = x.second;
            tracker.instances->Commit(session, [&](WGPUBuffer newBuffer) {
                tracker.pipeline->SetBinding(2, 0, newBuffer);
                tracker.pipeline->CommitBindings();
            });
        }
        session->Commit();
    }

    unsigned int ceildiv(unsigned int a, unsigned int b) {
        return (a + (b - 1)) / b;
    }
    void OptimizedDrawTracker::RenderStart(pipeline::wgpu::RenderBundle &bundle) {
        for (auto& x : trackers) {
            auto& tracker = x.second;
            unsigned int meshletGroupsCount = ceildiv(tracker.mesh->GetMeshletCount(), 96);
            tracker.pipeline->DispatchMeshTasks(bundle, meshletGroupsCount, tracker.instances->GetSize(), 1, nullptr);
            tracker.instances->Clear();
        }
    }

    void OptimizedDrawTracker::Draw(const std::shared_ptr<mesh::OptimizedMesh>& mesh, OptimizedMeshInstance &instance) {
        auto id = mesh->GetId();

        if (!trackers.contains(id)) {
            trackers[id] = OptimizedInstanceTracker {
                .instances = GetRenderer()->CreateBuffer<OptimizedMeshInstance>(std::format("OptMesh {} instances", id), WGPUBufferUsage_Storage, 64),
                .mesh = mesh,
                .pipeline = pipeline->CreateInstance()
            };
            trackers[id].pipeline->SetBinding(1, 0, mesh->GetMeshlets());
            trackers[id].pipeline->SetBinding(1, 1, mesh->GetVertices());
            trackers[id].pipeline->SetBinding(1, 2, mesh->GetIndices());
            trackers[id].pipeline->SetBinding(2, 0, *trackers[id].instances);
            trackers[id].pipeline->CommitBindings();
        }

        trackers[id].instances->Push(instance);
    }
}
