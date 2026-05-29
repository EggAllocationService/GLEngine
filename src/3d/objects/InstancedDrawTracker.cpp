//
// Created by Kyle Smith on 2026-05-29.
//

#include "3d/objects/InstancedDrawTracker.h"

namespace glengine::world::objects {
    InstancedDrawTracker::InstancedDrawTracker() {
        renderer = GetRenderer();

        pipeline = renderer->GetRenderPipelineByName("BasicLitInstanced");
    }

    void InstancedDrawTracker::UpdateEnd(double deltaTime) {
        for (auto& buf : buffers) {
            buf.second.data->Commit([&](WGPUBuffer buffer) {
                // handle buffer resize
                auto entry = WGPU_BIND_GROUP_ENTRY_INIT;
                entry.buffer = buffer;

                buf.second.pipeline->SetBinding(1, entry);
                buf.second.pipeline->CommitBindings();
            });
        }
    }

    void InstancedDrawTracker::RenderStart(pipeline::wgpu::RenderBundle &bundle) {
        for (auto& mesh : buffers) {
            auto& tracker = mesh.second;
            tracker.pipeline->DrawMeshInstanced(bundle, *tracker.mesh, tracker.data->GetSize());
            tracker.data->Clear();
        }
    }

    void InstancedDrawTracker::Draw(std::shared_ptr<pipeline::wgpu::GPUMesh> &mesh,
        instanced::InstanceData data) {

        auto id = mesh->GetId();
        if (!buffers.contains(id)) {
            auto tracker = instanced::InstanceTracker {
                .data = renderer->CreateBuffer<instanced::InstanceData>(std::format("Mesh {} instance data", id), WGPUBufferUsage_Storage, 128),
                .pipeline = pipeline->CreateInstance(),
                .mesh = mesh
            };

            auto entry = WGPU_BIND_GROUP_ENTRY_INIT;
            entry.buffer = *tracker.data;

            tracker.pipeline->SetBinding(1, entry);
            tracker.pipeline->CommitBindings();

            buffers.try_emplace(id, std::move(tracker));
        }

        buffers.at(id).data->Push(data);
    }
}