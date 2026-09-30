//
// Created by Kyle Smith on 2026-09-30.
//

#include "MeshDemoComponent.h"
#include "DemoShaders.h"

#include "Engine.h"

MeshDemoComponent::MeshDemoComponent() {
    pipeline_ = GetEngine()->GetRenderer()->GetMeshPipelineByName("DemoMesh");
}

void MeshDemoComponent::Render(const glengine::pipeline::wgpu::RenderBundle &bundle, glengine::MatrixStack &stack) {

    mat4 m = stack;
    pipeline_->DispatchMeshTasks(bundle, 1, 1, 1, &m);
}

void MeshDemoComponent::RegisterPipeline(glengine::Engine *engine) {
    auto renderer = engine->GetRenderer();
    auto shaders = renderer->CompileShader(embed_mesh_wgsl);
    renderer->BuildMeshPipeline(
        "DemoMesh",
        shaders,
        false,
        {},
        sizeof(mat4),
        nullptr
    );
}
