//
// Created by Kyle Smith on 2026-05-30.
//

#include "3d/objects/LightTracker.h"

glengine::world::objects::LightTracker::LightTracker() {
    auto renderer = GetRenderer();
    lights = renderer->CreateBuffer<rendering::LightInfo>("GLEngine: Light Info", WGPUBufferUsage_Storage, 10);

    WGPUBindGroupEntry entry = WGPU_BIND_GROUP_ENTRY_INIT;
    entry.binding = 1;
    entry.buffer = *lights;
    renderer->SetUniversalBindGroupEntry(entry);
}

void glengine::world::objects::LightTracker::UpdateEnd(double deltaTime) {
    lights->Commit([](WGPUBuffer newBuf) {
        WGPUBindGroupEntry entry = WGPU_BIND_GROUP_ENTRY_INIT;
        entry.binding = 1;
        entry.buffer = newBuf;
        GetRenderer()->SetUniversalBindGroupEntry(entry);
    });
}

void glengine::world::objects::LightTracker::AddLight(rendering::LightInfo info) {
    lights->Push(info);
}

void glengine::world::objects::LightTracker::RenderStart(pipeline::wgpu::RenderBundle &bundle) {
    lights->Clear();
}
